#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cuteid.h"

static const char *variant_names[] = {"ncs", "rfc9562", "microsoft", "future"};
static const char *version_names[16] = {
	[0] = "nil",          [1] = "time-greg",   [2] = "dce",
	[3] = "name-md5",     [4] = "random",      [5] = "name-sha1",
	[6] = "time-reorder", [7] = "time-unix",   [8] = "custom",
	[15] = "max",
};

static void usage(void)
{
	dprintf(2,
		"usage: cuteid <command> [args]\n"
		"\n"
		"commands:\n"
		"  encode <server_id> <unix_ts> <process_handle> [alert_handle]\n"
		"  decode [uuid]\n"
		"  info   [uuid]\n"
		"\n"
		"uuid is read from stdin when omitted or '-'\n");
}

static int read_uuid_arg(struct uuid *u, const char *arg)
{
	char buf[64];
	ssize_t n;

	if (!arg || !strcmp(arg, "-")) {
		n = read(0, buf, sizeof(buf) - 1);
		if (n <= 0) {
			dprintf(2, "cuteid: failed to read uuid from stdin\n");
			return -1;
		}
		buf[n] = '\0';
	} else {
		if (strlen(arg) >= sizeof(buf)) {
			dprintf(2, "cuteid: input too long\n");
			return -1;
		}
		strcpy(buf, arg);
	}

	if (uuid_parse(u, buf) < 0) {
		dprintf(2, "cuteid: invalid uuid\n");
		return -1;
	}
	return 0;
}

static int cmd_encode(int argc, char **argv)
{
	struct cuteid_fields f = {0};
	struct uuid u;
	char out[40];
	unsigned long val;
	unsigned long long unix_ts;

	if (argc < 4) {
		dprintf(2, "usage: cuteid encode <server_id> <unix_ts> <process_handle> [alert_handle]\n");
		return 1;
	}

	val = strtoul(argv[1], NULL, 0);
	if (val > 15) {
		dprintf(2, "cuteid: server_id must be 0-15\n");
		return 1;
	}
	f.server_id = val;

	unix_ts = strtoull(argv[2], NULL, 0);
	if (unix_ts < CUTEID_EPOCH) {
		dprintf(2, "cuteid: timestamp before epoch (2026-01-01)\n");
		return 1;
	}
	f.start_time = (unix_ts - CUTEID_EPOCH) * 1000;
	if (f.start_time > 0x3FFFFFFFFFFULL) {
		dprintf(2, "cuteid: timestamp out of 42-bit range\n");
		return 1;
	}

	f.process_handle = strtoul(argv[3], NULL, 0);
	f.alert_handle = (argc > 4) ? strtoul(argv[4], NULL, 0) : 0;
	f.schema = CUTEID_SCHEMA;

	cuteid_encode(&u, &f);
	uuid_format(out, &u, 1);
	dprintf(1, "%s\n", out);
	return 0;
}

static int cmd_decode(int argc, char **argv)
{
	struct uuid u;
	struct cuteid_fields f;
	unsigned long long unix_ts;

	if (read_uuid_arg(&u, argc > 1 ? argv[1] : NULL) < 0)
		return 1;

	if (uuid_version(&u) != 8) {
		dprintf(2, "cuteid: not a v8 uuid (version=%d)\n", uuid_version(&u));
		return 1;
	}

	cuteid_decode(&f, &u);

	if (f.schema != CUTEID_SCHEMA) {
		dprintf(2, "cuteid: unknown schema %u\n", f.schema);
		return 1;
	}

	unix_ts = CUTEID_EPOCH + f.start_time / 1000;

	dprintf(1, "server_id=%u\n", f.server_id);
	dprintf(1, "start_time=%llu\n", unix_ts);
	dprintf(1, "process_handle=%u\n", f.process_handle);
	dprintf(1, "alert_handle=%u\n", f.alert_handle);
	dprintf(1, "type=%s\n", f.alert_handle ? "alert" : "process");
	return 0;
}

static int cmd_info(int argc, char **argv)
{
	struct uuid u;
	char formatted[40];
	int ver, var;
	const char *vname;

	if (read_uuid_arg(&u, argc > 1 ? argv[1] : NULL) < 0)
		return 1;

	ver = uuid_version(&u);
	var = uuid_variant(&u);
	uuid_format(formatted, &u, 1);

	dprintf(1, "uuid=%s\n", formatted);

	vname = (ver >= 0 && ver <= 15) ? version_names[ver] : NULL;
	dprintf(1, "version=%d", ver);
	if (vname)
		dprintf(1, " (%s)", vname);
	dprintf(1, "\n");

	dprintf(1, "variant=%d (%s)\n", var, variant_names[var]);

	/* if it looks like one of ours, decode the entity fields */
	if (ver == 8 && var == 1) {
		struct cuteid_fields f;
		unsigned long long unix_ts;
		time_t ts;
		struct tm tm;
		char tbuf[32];

		cuteid_decode(&f, &u);
		if (f.schema == CUTEID_SCHEMA) {
			unix_ts = CUTEID_EPOCH + f.start_time / 1000;
			ts = (time_t)unix_ts;
			gmtime_r(&ts, &tm);
			strftime(tbuf, sizeof(tbuf), "%Y-%m-%dT%H:%M:%SZ", &tm);

			dprintf(1, "schema=%u\n", f.schema);
			dprintf(1, "server_id=%u\n", f.server_id);
			dprintf(1, "start_time=%llu (%s)\n", unix_ts, tbuf);
			dprintf(1, "process_handle=%u\n", f.process_handle);
			dprintf(1, "alert_handle=%u\n", f.alert_handle);
			dprintf(1, "type=%s\n", f.alert_handle ? "alert" : "process");
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
		return 1;
	}

	if (!strcmp(argv[1], "encode")) return cmd_encode(argc - 1, argv + 1);
	if (!strcmp(argv[1], "decode")) return cmd_decode(argc - 1, argv + 1);
	if (!strcmp(argv[1], "info"))   return cmd_info(argc - 1, argv + 1);
	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		usage();
		return 0;
	}

	dprintf(2, "cuteid: unknown command '%s'\n", argv[1]);
	usage();
	return 1;
}
