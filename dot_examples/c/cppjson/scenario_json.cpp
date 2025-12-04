#include "scenario_json.h"
#include "simdjson.h"

#include <string_view>
#include <string.h>
#include <stdint.h>

static int sv_eq_icase(std::string_view v, const char *lit)
{
	size_t i;
	size_t n;
	unsigned char ca;
	unsigned char cb;
	n = v.size();
	for (i = 0; i < n && lit[i] != 0; i++) {
		ca = (unsigned char)v[i];
		cb = (unsigned char)lit[i];
		if (ca >= 'A' && ca <= 'Z')
			ca = (unsigned char)(ca - 'A' + 'a');
		if (cb >= 'A' && cb <= 'Z')
			cb = (unsigned char)(cb - 'A' + 'a');
		if (ca != cb)
			return 0;
	}
	if (i != n)
		return 0;
	if (lit[i] != 0)
		return 0;
	return 1;
}

static app_status_t parse_app_status_sv(std::string_view v)
{
	if (sv_eq_icase(v, "Unknown"))
		return SC_APP_STATUS_UNKNOWN;
	if (sv_eq_icase(v, "Up"))
		return SC_APP_STATUS_UP;
	if (sv_eq_icase(v, "Weak"))
		return SC_APP_STATUS_WEAK;
	if (sv_eq_icase(v, "Dead"))
		return SC_APP_STATUS_DEAD;
	if (sv_eq_icase(v, "Ended"))
		return SC_APP_STATUS_ENDED;
	return SC_APP_STATUS_UNKNOWN;
}

static alert_level_t parse_alert_level_sv(std::string_view v)
{
	if (sv_eq_icase(v, "Unknown"))
		return SC_ALERT_LEVEL_UNKNOWN;
	if (sv_eq_icase(v, "Positive"))
		return SC_ALERT_LEVEL_POSITIVE;
	if (sv_eq_icase(v, "Information"))
		return SC_ALERT_LEVEL_INFORMATION;
	if (sv_eq_icase(v, "Warning"))
		return SC_ALERT_LEVEL_WARNING;
	if (sv_eq_icase(v, "Critical"))
		return SC_ALERT_LEVEL_CRITICAL;
	if (sv_eq_icase(v, "Fatal"))
		return SC_ALERT_LEVEL_FATAL;
	return SC_ALERT_LEVEL_UNKNOWN;
}

static int arena_copy_sv(std::string_view v, char *buf, size_t cap, size_t *used,
			 const char **out_ptr)
{
	size_t need;
	char *dst;
	need = v.size() + 1u;
	if (*used + need > cap)
		return -1;
	dst = buf + *used;
	if (v.size() > 0)
		memcpy(dst, v.data(), v.size());
	dst[v.size()] = 0;
	*out_ptr = dst;
	*used += need;
	return 0;
}

static int parse_labels_array(simdjson::dom::array arr, scenario_t *sc, char *buf, size_t cap,
			      size_t *used, uint32_t *label_start, uint32_t *label_count)
{
	uint32_t base;
	uint32_t count;
	simdjson::dom::element elem;
	std::string_view sv;
	simdjson::error_code ec;

	base = sc->label_count;
	count = 0;

	for (simdjson::dom::array::iterator it = arr.begin(); it != arr.end(); ++it) {
		elem = *it;
		ec = elem.get(sv);
		if (ec)
			return -1;
		if (sc->label_count >= SCENARIO_MAX_LABELS)
			return -1;
		if (arena_copy_sv(sv, buf, cap, used, &sc->labels[sc->label_count].value) != 0)
			return -1;
		sc->label_count++;
		count++;
	}

	*label_start = base;
	*label_count = count;
	return 0;
}

static int parse_app(simdjson::dom::object obj, scenario_t *sc, char *buf, size_t cap, size_t *used)
{
	simdjson::error_code ec;
	uint64_t handle;
	std::string_view sv;
	simdjson::dom::array arr;
	const char *p;

	sc->app.handle = 1;
	sc->app.status = SC_APP_STATUS_UP;
	sc->app.name = NULL;
	sc->app.host_name = NULL;
	sc->app.label_start = 0;
	sc->app.label_count = 0;

	ec = obj["handle"].get(handle);
	if (!ec)
		sc->app.handle = handle;

	ec = obj["name"].get(sv);
	if (ec)
		return -1;
	if (arena_copy_sv(sv, buf, cap, used, &p) != 0)
		return -1;
	sc->app.name = p;

	ec = obj["host_name"].get(sv);
	if (!ec) {
		if (arena_copy_sv(sv, buf, cap, used, &p) != 0)
			return -1;
		sc->app.host_name = p;
	}

	ec = obj["status"].get(sv);
	if (!ec)
		sc->app.status = parse_app_status_sv(sv);

	ec = obj["labels"].get(arr);
	if (!ec) {
		if (parse_labels_array(arr, sc, buf, cap, used, &sc->app.label_start,
				       &sc->app.label_count) != 0)
			return -1;
	}

	return 0;
}

static int parse_alert_defaults(simdjson::dom::object obj, scenario_t *sc, char *buf, size_t cap,
				size_t *used)
{
	simdjson::error_code ec;
	std::string_view sv;
	simdjson::dom::array arr;

	sc->has_default_level = 0;
	sc->default_label_start = sc->label_count;
	sc->default_label_count = 0;

	ec = obj["level"].get(sv);
	if (!ec) {
		sc->default_level = parse_alert_level_sv(sv);
		sc->has_default_level = 1;
	}

	ec = obj["labels"].get(arr);
	if (!ec) {
		if (parse_labels_array(arr, sc, buf, cap, used, &sc->default_label_start,
				       &sc->default_label_count) != 0)
			return -1;
	}

	return 0;
}

static int parse_single_alert(simdjson::dom::object obj, scenario_t *sc, alert_spec_t *dst,
			      char *buf, size_t cap, size_t *used)
{
	simdjson::error_code ec;
	std::string_view sv;
	simdjson::dom::array arr;
	uint64_t at;
	uint64_t count64;
	uint64_t period64;
	const char *p;

	dst->title = NULL;
	dst->body = NULL;
	dst->level = SC_ALERT_LEVEL_UNKNOWN;
	dst->label_start = 0;
	dst->label_count = 0;
	dst->count = 1;
	dst->period_ms = 0;
	dst->has_at_ms = 0;
	dst->at_ms = 0;

	ec = obj["title"].get(sv);
	if (ec)
		return -1;
	if (arena_copy_sv(sv, buf, cap, used, &p) != 0)
		return -1;
	dst->title = p;

	ec = obj["body"].get(sv);
	if (!ec) {
		if (arena_copy_sv(sv, buf, cap, used, &p) != 0)
			return -1;
		dst->body = p;
	}

	ec = obj["level"].get(sv);
	if (!ec)
		dst->level = parse_alert_level_sv(sv);

	ec = obj["labels"].get(arr);
	if (!ec) {
		if (parse_labels_array(arr, sc, buf, cap, used, &dst->label_start,
				       &dst->label_count) != 0)
			return -1;
	}

	ec = obj["count"].get(count64);
	if (!ec) {
		if (count64 == 0 || count64 > 0xffffffffu)
			return -1;
		dst->count = (uint32_t)count64;
	}

	ec = obj["period_ms"].get(period64);
	if (!ec) {
		if (period64 > 0xffffffffu)
			return -1;
		dst->period_ms = (uint32_t)period64;
	}

	ec = obj["at_ms"].get(at);
	if (!ec) {
		dst->has_at_ms = 1;
		dst->at_ms = at;
	}

	return 0;
}

int scenario_load_from_json(const char *json, size_t len, scenario_t *out, char *string_buf,
			    size_t string_buf_cap, size_t *string_buf_used)
{
	simdjson::dom::parser parser;
	simdjson::dom::element root;
	simdjson::dom::object obj;
	simdjson::dom::array arr;
	simdjson::error_code ec;
	scenario_t s;
	size_t used;
	uint32_t idx;
	simdjson::dom::element el;

	if (!json || !out || !string_buf || !string_buf_used)
		return -1;

	memset(&s, 0, sizeof(s));
	used = 0;

	ec = parser.parse(json, len).get(root);
	if (ec)
		return -1;

	ec = root["app"].get(obj);
	if (ec)
		return -1;
	if (parse_app(obj, &s, string_buf, string_buf_cap, &used) != 0)
		return -1;

	ec = root["alert_defaults"].get(obj);
	if (!ec) {
		if (parse_alert_defaults(obj, &s, string_buf, string_buf_cap, &used) != 0)
			return -1;
	} else {
		s.has_default_level = 0;
		s.default_label_start = s.label_count;
		s.default_label_count = 0;
	}

	s.alert_count = 0;

	ec = root["alerts"].get(arr);
	if (!ec) {
		idx = 0;
		for (simdjson::dom::array::iterator it = arr.begin(); it != arr.end(); ++it) {
			el = *it;
			ec = el.get(obj);
			if (ec)
				return -1;
			if (idx >= SCENARIO_MAX_ALERTS)
				return -1;
			if (parse_single_alert(obj, &s, &s.alerts[idx], string_buf, string_buf_cap,
					       &used) != 0)
				return -1;
			idx++;
		}
		s.alert_count = idx;
	}

	*out = s;
	*string_buf_used = used;
	return 0;
}
