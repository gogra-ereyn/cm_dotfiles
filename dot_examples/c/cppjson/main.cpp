#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <string>

extern "C" {
#include "scenario.h"
}
#include "scenario_json.h"

#include "event.pb.h"
#include <google/protobuf/text_format.h>
#include <google/protobuf/stubs/common.h>

static int read_all_fd(int fd, char *buf, size_t cap, size_t *out_len)
{
	size_t off;
	ssize_t n;

	off = 0;
	for (;;) {
		if (off >= cap)
			return -1;
		n = read(fd, buf + off, cap - off);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (n == 0)
			break;
		off += (size_t)n;
	}
	buf[off] = 0;
	*out_len = off;
	return 0;
}

static int read_config(const char *path, char *buf, size_t cap, size_t *out_len)
{
	int fd;
	struct stat st;
	int r;

	if (!path || !buf || !out_len)
		return -1;

	if (path[0] == '-' && path[1] == 0) {
		r = read_all_fd(STDIN_FILENO, buf, cap - 1u, out_len);
		return r;
	}

	if (stat(path, &st) != 0)
		return -1;
	if (!S_ISREG(st.st_mode))
		return -1;
	if ((uint64_t)st.st_size > (uint64_t)(cap - 1u))
		return -1;

	fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return -1;

	r = read_all_fd(fd, buf, cap - 1u, out_len);
	close(fd);
	return r;
}

static void emit_event_text(const Event &ev)
{
	std::string out;
	bool ok;

	ok = google::protobuf::TextFormat::PrintToString(ev, &out);
	if (!ok)
		return;

	fwrite(out.data(), 1u, out.size(), stdout);
	fputc('\n', stdout);
}

static void build_and_emit_application(const scenario_t *sc)
{
	Event ev;
	Application *app;
	uint32_t i;
	uint32_t idx;

	ev.set_type(EVENT_TYPE_APPLICATION);
	ev.set_app_handle(sc->app.handle);
	ev.set_send_time_ms(0);
	ev.set_receive_time_ms(0);

	app = ev.mutable_application();
	if (sc->app.name)
		app->set_name(sc->app.name);
	if (sc->app.host_name)
		app->set_host_name(sc->app.host_name);

	if (sc->app.status == SC_APP_STATUS_UP)
		app->set_status(APPLICATION_STATUS_UP);
	else if (sc->app.status == SC_APP_STATUS_WEAK)
		app->set_status(APPLICATION_STATUS_WEAK);
	else if (sc->app.status == SC_APP_STATUS_DEAD)
		app->set_status(APPLICATION_STATUS_DEAD);
	else if (sc->app.status == SC_APP_STATUS_ENDED)
		app->set_status(APPLICATION_STATUS_ENDED);
	else
		app->set_status(APPLICATION_STATUS_UNKNOWN);

	for (i = 0; i < sc->app.label_count; i++) {
		idx = sc->app.label_start + i;
		if (idx < sc->label_count && sc->labels[idx].value) {
			app->add_labels(sc->labels[idx].value);
		}
	}

	emit_event_text(ev);
}

static AlertLevel map_alert_level(alert_level_t lvl)
{
	if (lvl == SC_ALERT_LEVEL_POSITIVE)
		return ALERT_LEVEL_POSITIVE;
	if (lvl == SC_ALERT_LEVEL_INFORMATION)
		return ALERT_LEVEL_INFORMATION;
	if (lvl == SC_ALERT_LEVEL_WARNING)
		return ALERT_LEVEL_WARNING;
	if (lvl == SC_ALERT_LEVEL_CRITICAL)
		return ALERT_LEVEL_CRITICAL;
	if (lvl == SC_ALERT_LEVEL_FATAL)
		return ALERT_LEVEL_FATAL;
	return ALERT_LEVEL_UNKNOWN;
}

static void build_and_emit_alerts(const scenario_t *sc)
{
	uint32_t i;
	uint32_t j;
	uint32_t k;
	uint32_t idx;
	const alert_spec_t *aspec;

	for (i = 0; i < sc->alert_count; i++) {
		aspec = &sc->alerts[i];

		for (j = 0; j < aspec->count; j++) {
			Event ev;
			Alert *alert;

			ev.set_type(EVENT_TYPE_ALERT);
			ev.set_app_handle(sc->app.handle);
			ev.set_send_time_ms(0);
			ev.set_receive_time_ms(0);

			alert = ev.mutable_alert();

			if (aspec->title)
				alert->set_title(aspec->title);
			if (aspec->body)
				alert->set_body(aspec->body);

			alert->set_level(map_alert_level(aspec->level));

			for (k = 0; k < sc->default_label_count; k++) {
				idx = sc->default_label_start + k;
				if (idx < sc->label_count && sc->labels[idx].value) {
					alert->add_labels(sc->labels[idx].value);
				}
			}

			for (k = 0; k < aspec->label_count; k++) {
				idx = aspec->label_start + k;
				if (idx < sc->label_count && sc->labels[idx].value) {
					alert->add_labels(sc->labels[idx].value);
				}
			}

			emit_event_text(ev);
		}
	}
}

int main(int argc, char **argv)
{
	const char *config_path;
	char cfg_buf[SCENARIO_MAX_FILE_SIZE + 1u];
	size_t cfg_len;
	scenario_t sc;
	char arena[64u * 1024u];
	size_t arena_used;
	int rc;

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	config_path = "-";
	if (argc >= 2) {
		config_path = argv[1];
    } else {
        fprintf(stderr, "Config path not provided, will try read from stdin...");
    }

	rc = read_config(config_path, cfg_buf, sizeof cfg_buf, &cfg_len);
	if (rc != 0) {
		fprintf(stderr, "failed to read config from %s\n", config_path);
		google::protobuf::ShutdownProtobufLibrary();
		return 1;
	}

	arena_used = 0;
	rc = scenario_load_from_json(cfg_buf, cfg_len, &sc, arena, sizeof arena, &arena_used);
	if (rc != 0) {
		fprintf(stderr, "failed to parse scenario json\n");
		google::protobuf::ShutdownProtobufLibrary();
		return 1;
	}

	build_and_emit_application(&sc);
	build_and_emit_alerts(&sc);

	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
