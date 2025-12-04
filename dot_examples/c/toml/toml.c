#define _GNU_SOURCE
#include "scenario.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

typedef enum {
	SECTION_NONE = 0,
	SECTION_APP,
	SECTION_ALERT_DEFAULTS,
	SECTION_ALERT
} section_t;

static int is_space_char(int c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static char *trim_left(char *s, char *end)
{
	while (s < end && is_space_char(*s))
		s++;
	return s;
}

static char *trim_right(char *s, char *end)
{
	while (end > s && is_space_char(*(end - 1)))
		end--;
	return end;
}

static int str_eq_nocase(const char *a, const char *b)
{
	unsigned char ca;
	unsigned char cb;
	while (*a && *b) {
		ca = *a;
		cb = *b;
		if (ca >= 'A' && ca <= 'Z')
			ca = (ca - 'A' + 'a');
		if (cb >= 'A' && cb <= 'Z')
			cb = (cb - 'A' + 'a');
		if (ca != cb)
			return 0;
		a++;
		b++;
	}
	return *a == 0 && *b == 0;
}

static app_status_t parse_app_status(const char *s)
{
	if (str_eq_nocase(s, "Unknown"))
		return APP_STATUS_UNKNOWN;
	if (str_eq_nocase(s, "Up"))
		return APP_STATUS_UP;
	if (str_eq_nocase(s, "Weak"))
		return APP_STATUS_WEAK;
	if (str_eq_nocase(s, "Dead"))
		return APP_STATUS_DEAD;
	if (str_eq_nocase(s, "Ended"))
		return APP_STATUS_ENDED;
	return APP_STATUS_UNKNOWN;
}

static alert_level_t parse_alert_level(const char *s)
{
	if (str_eq_nocase(s, "Unknown"))
		return ALERT_LEVEL_UNKNOWN;
	if (str_eq_nocase(s, "Positive"))
		return ALERT_LEVEL_POSITIVE;
	if (str_eq_nocase(s, "Information"))
		return ALERT_LEVEL_INFORMATION;
	if (str_eq_nocase(s, "Warning"))
		return ALERT_LEVEL_WARNING;
	if (str_eq_nocase(s, "Critical"))
		return ALERT_LEVEL_CRITICAL;
	if (str_eq_nocase(s, "Fatal"))
		return ALERT_LEVEL_FATAL;
	return ALERT_LEVEL_UNKNOWN;
}

static int parse_uint64(const char *s, uint64_t *out)
{
	char *endp;
	unsigned long long v;
	errno = 0;
	v = strtoull(s, &endp, 10);
	if (errno != 0)
		return -1;
	if (endp == s)
		return -1;
	while (*endp && is_space_char((unsigned char)*endp))
		endp++;
	if (*endp != 0)
		return -1;
	*out = (uint64_t)v;
	return 0;
}

static int parse_uint32(const char *s, uint32_t *out)
{
	uint64_t v64;
	if (parse_uint64(s, &v64) != 0)
		return -1;
	if (v64 > 0xffffffffu)
		return -1;
	*out = (uint32_t)v64;
	return 0;
}

static int parse_string_literal(char *start, char *end, char **out)
{
	char *p;
	char *w;
	if (start >= end)
		return -1;
	if (*start != '"')
		return -1;
	p = start + 1;
	w = p;
	while (p < end) {
		if (*p == '\\') {
			if (p + 1 >= end)
				return -1;
			p++;
			if (*p == '"' || *p == '\\') {
				*w++ = *p++;
			} else {
				return -1;
			}
		} else if (*p == '"') {
			*w = 0;
			p++;
			while (p < end && is_space_char((unsigned char)*p))
				p++;
			if (p != end)
				return -1;
			*out = start + 1;
			return 0;
		} else {
			*w++ = *p++;
		}
	}
	return -1;
}

static int parse_string_array(char *start, char *end, scenario_t *sc,
			      uint32_t *label_start, uint32_t *label_count)
{
	char *p;
	char *elem_start;
	char *elem_end;
	uint32_t base;
	uint32_t count;
	base = sc->label_count;
	count = 0;
	p = start;
	if (*p != '[')
		return -1;
	p++;
	for (;;) {
		p = trim_left(p, end);
		if (p >= end)
			return -1;
		if (*p == ']') {
			p++;
			p = trim_left(p, end);
			if (p != end)
				return -1;
			break;
		}
		elem_start = p;
		while (p < end && *p != ',' && *p != ']')
			p++;
		elem_end = p;
		elem_start = trim_left(elem_start, elem_end);
		elem_end = trim_right(elem_start, elem_end);
		if (elem_start >= elem_end)
			return -1;
		if (sc->label_count >= SCENARIO_MAX_LABELS)
			return -1;
		if (*elem_start == '"' && elem_end > elem_start + 1 &&
		    *(elem_end - 1) == '"') {
			char *s;
			char *inner_end;
			inner_end = elem_end;
			if (parse_string_literal(elem_start, inner_end, &s) !=
			    0)
				return -1;
			sc->labels[sc->label_count].value = s;
		} else {
			*elem_end = 0;
			sc->labels[sc->label_count].value = elem_start;
		}
		sc->label_count++;
		count++;
		p = elem_end;
		p = trim_left(p, end);
		if (p >= end)
			return -1;
		if (*p == ',') {
			p++;
			continue;
		} else if (*p == ']') {
			p++;
			p = trim_left(p, end);
			if (p != end)
				return -1;
			break;
		} else {
			return -1;
		}
	}
	*label_start = base;
	*label_count = count;
	return 0;
}

static int parse_value_string_or_ident(char *val_start, char *val_end,
				       char **out)
{
	val_start = trim_left(val_start, val_end);
	val_end = trim_right(val_start, val_end);
	if (val_start >= val_end)
		return -1;
	if (*val_start == '"' && val_end > val_start + 1 &&
	    *(val_end - 1) == '"') {
		if (parse_string_literal(val_start, val_end, out) != 0)
			return -1;
		return 0;
	} else {
		*val_end = 0;
		*out = val_start;
		return 0;
	}
}

static int parse_section_header(char *line_start, char *line_end,
				section_t *section, scenario_t *sc,
				alert_spec_t **current_alert)
{
	char *p;
	char *inner_start;
	char *inner_end;
	p = trim_left(line_start, line_end);
	if (p >= line_end)
		return 0;
	if (*p != '[')
		return 0;
	if (p + 1 < line_end && *(p + 1) == '[') {
		p += 2;
		inner_start = p;
		while (p < line_end && *p != ']')
			p++;
		if (p >= line_end || p + 1 >= line_end || *p != ']' ||
		    *(p + 1) != ']')
			return -1;
		inner_end = p;
		inner_start = trim_left(inner_start, inner_end);
		inner_end = trim_right(inner_start, inner_end);
		if (inner_start >= inner_end)
			return -1;
		if ((inner_end - inner_start) == 6 &&
		    strncasecmp(inner_start, "alerts", 6) == 0) {
			if (sc->alert_count >= SCENARIO_MAX_ALERTS)
				return -1;
			*section = SECTION_ALERT;
			*current_alert = &sc->alerts[sc->alert_count];
			memset(*current_alert, 0, sizeof(alert_spec_t));
			(*current_alert)->level = ALERT_LEVEL_UNKNOWN;
			(*current_alert)->count = 1;
			(*current_alert)->period_ms = 0;
			(*current_alert)->has_at_ms = 0;
			sc->alert_count++;
			return 1;
		}
		return -1;
	} else {
		p++;
		inner_start = p;
		while (p < line_end && *p != ']')
			p++;
		if (p >= line_end || *p != ']')
			return -1;
		inner_end = p;
		inner_start = trim_left(inner_start, inner_end);
		inner_end = trim_right(inner_start, inner_end);
		if (inner_start >= inner_end)
			return -1;
		if ((inner_end - inner_start) == 3 &&
		    strncasecmp(inner_start, "app", 3) == 0) {
			*section = SECTION_APP;
			memset(&sc->app, 0, sizeof(sc->app));
			sc->app.handle = 1;
			sc->app.status = APP_STATUS_UP;
			return 1;
		} else if ((inner_end - inner_start) == 14 &&
			   strncasecmp(inner_start, "alert_defaults", 14) ==
				   0) {
			*section = SECTION_ALERT_DEFAULTS;
			sc->has_default_level = 0;
			sc->default_label_start = sc->label_count;
			sc->default_label_count = 0;
			return 1;
		}
		return -1;
	}
}

static int handle_kv_app(char *key, char *val_start, char *val_end,
			 scenario_t *sc)
{
	if (str_eq_nocase(key, "handle")) {
		uint64_t v;
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		if (parse_uint64(s, &v) != 0)
			return -1;
		sc->app.handle = v;
		return 0;
	}
	if (str_eq_nocase(key, "name")) {
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		sc->app.name = s;
		return 0;
	}
	if (str_eq_nocase(key, "host_name")) {
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		sc->app.host_name = s;
		return 0;
	}
	if (str_eq_nocase(key, "status")) {
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		sc->app.status = parse_app_status(s);
		return 0;
	}
	if (str_eq_nocase(key, "labels")) {
		uint32_t start;
		uint32_t count;
		if (parse_string_array(val_start, val_end, sc, &start,
				       &count) != 0)
			return -1;
		sc->app.label_start = start;
		sc->app.label_count = count;
		return 0;
	}
	return 0;
}

static int handle_kv_alert_defaults(char *key, char *val_start, char *val_end,
				    scenario_t *sc)
{
	if (str_eq_nocase(key, "level")) {
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		sc->default_level = parse_alert_level(s);
		sc->has_default_level = 1;
		return 0;
	}
	if (str_eq_nocase(key, "labels")) {
		uint32_t start;
		uint32_t count;
		if (parse_string_array(val_start, val_end, sc, &start,
				       &count) != 0)
			return -1;
		sc->default_label_start = start;
		sc->default_label_count = count;
		return 0;
	}
	return 0;
}

static int handle_kv_alert(char *key, char *val_start, char *val_end,
			   scenario_t *sc, alert_spec_t *cur)
{
	if (str_eq_nocase(key, "title")) {
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		cur->title = s;
		return 0;
	}
	if (str_eq_nocase(key, "body")) {
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		cur->body = s;
		return 0;
	}
	if (str_eq_nocase(key, "level")) {
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		cur->level = parse_alert_level(s);
		return 0;
	}
	if (str_eq_nocase(key, "labels")) {
		uint32_t start;
		uint32_t count;
		if (parse_string_array(val_start, val_end, sc, &start,
				       &count) != 0)
			return -1;
		cur->label_start = start;
		cur->label_count = count;
		return 0;
	}
	if (str_eq_nocase(key, "count")) {
		uint32_t v;
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		if (parse_uint32(s, &v) != 0)
			return -1;
		if (v == 0)
			return -1;
		cur->count = v;
		return 0;
	}
	if (str_eq_nocase(key, "period_ms")) {
		uint32_t v;
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		if (parse_uint32(s, &v) != 0)
			return -1;
		cur->period_ms = v;
		return 0;
	}
	if (str_eq_nocase(key, "at_ms")) {
		uint64_t v;
		char *s;
		if (parse_value_string_or_ident(val_start, val_end, &s) != 0)
			return -1;
		if (parse_uint64(s, &v) != 0)
			return -1;
		cur->has_at_ms = 1;
		cur->at_ms = v;
		return 0;
	}
	return 0;
}

static int parse_buffer(char *buf, uint32_t len, scenario_t *sc)
{
	uint32_t i;
	char *line_start;
	char *line_end;
	section_t section;
	alert_spec_t *current_alert;
	memset(sc, 0, sizeof(*sc));
	sc->app.handle = 1;
	sc->app.status = APP_STATUS_UP;
	section = SECTION_NONE;
	current_alert = 0;
	i = 0;
	line_start = buf;
	while (i <= len) {
		if (i == len || buf[i] == '\n') {
			line_end = &buf[i];
			{
				char *p;
				char *q;
				p = trim_left(line_start, line_end);
				q = trim_right(p, line_end);
				if (p < q && *p != '#') {
					int sec_res;
					sec_res = parse_section_header(
						p, q, &section, sc,
						&current_alert);
					if (sec_res < 0)
						return -1;
					if (sec_res == 0) {
						char *eq;
						char *key_start;
						char *key_end;
						char *val_start;
						char *val_end;
						eq = memchr(p, '=',
							    (size_t)(q - p));
						if (!eq)
							return -1;
						key_start = p;
						key_end = eq;
						key_start = trim_left(key_start,
								      key_end);
						key_end = trim_right(key_start,
								     key_end);
						if (key_start >= key_end)
							return -1;
						*key_end = 0;
						val_start = eq + 1;
						val_end = q;
						if (section == SECTION_APP) {
							if (handle_kv_app(
								    key_start,
								    val_start,
								    val_end,
								    sc) != 0)
								return -1;
						} else if (section ==
							   SECTION_ALERT_DEFAULTS) {
							if (handle_kv_alert_defaults(
								    key_start,
								    val_start,
								    val_end,
								    sc) != 0)
								return -1;
						} else if (section ==
								   SECTION_ALERT &&
							   current_alert != 0) {
							if (handle_kv_alert(
								    key_start,
								    val_start,
								    val_end, sc,
								    current_alert) !=
							    0)
								return -1;
						} else {
							return -1;
						}
					}
				}
			}
			if (i == len)
				break;
			line_start = &buf[i + 1];
		}
		i++;
	}
	return 0;
}

static int read_all_fd(int fd, char **out_buf, uint32_t *out_len)
{
	char *buf;
	uint32_t cap;
	uint32_t off;
	ssize_t n;
	buf = malloc(SCENARIO_MAX_FILE_SIZE + 1u);
	if (!buf)
		return -1;
	cap = SCENARIO_MAX_FILE_SIZE;
	off = 0;
	for (;;) {
		if (off >= cap) {
			free(buf);
			return -1;
		}
		n = read(fd, buf + off, (size_t)(cap - off));
		if (n < 0) {
			if (errno == EINTR)
				continue;
			free(buf);
			return -1;
		}
		if (n == 0)
			break;
		off += (uint32_t)n;
	}
	buf[off] = 0;
	*out_buf = buf;
	*out_len = off;
	return 0;
}

int scenario_load_from_file(const char *path, scenario_t *out, char **out_buf,
			    uint32_t *out_len)
{
	int fd;
	int r;
	char *buf;
	uint32_t len;
	if (!path || !out || !out_buf || !out_len)
		return -1;
	buf = 0;
	len = 0;
	if (path[0] == '-' && path[1] == 0) {
		if (read_all_fd(STDIN_FILENO, &buf, &len) != 0)
			return -1;
	} else {
		struct stat st;
		if (stat(path, &st) != 0)
			return -1;
		if (!S_ISREG(st.st_mode))
			return -1;
		if ((uint64_t)st.st_size > SCENARIO_MAX_FILE_SIZE)
			return -1;
		fd = open(path, O_RDONLY | O_CLOEXEC);
		if (fd < 0)
			return -1;
		if (read_all_fd(fd, &buf, &len) != 0) {
			close(fd);
			return -1;
		}
		close(fd);
	}
	r = parse_buffer(buf, len, out);
	if (r != 0) {
		free(buf);
		return -1;
	}
	*out_buf = buf;
	*out_len = len;
	return 0;
}
