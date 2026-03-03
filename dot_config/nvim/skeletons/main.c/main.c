#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int use_color(void)
{
	const char *nc;
	nc = getenv("NO_COLOR");
	if (nc && *nc)
		return 0;
	if (!isatty(2))
		return 0;
	return 1;
}

static inline char *envstr(char *name)
{
	return getenv(name) ?: "";
}

static void t_error(int status, int errnum, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(stderr, format, args);
	if (errnum)
		fprintf(stderr, ": %s", strerror(errnum));
	fprintf(stderr, "\n");
	va_end(args);
	exit(status);
}

static int usage(char *name, int rc)
{
	dprintf(2, "Usage: %s [options]\n", name);
	dprintf(2, "Options:\n");

	dprintf(2, "\t-h, --help\n");
	dprintf(2, "\t\tShow this help message\n");
	dprintf(2, "\n");

	dprintf(2, "\t-a, --address <ADDRESS>\n");
	dprintf(2, "\t\tInterface address to bind to.\n");
	dprintf(2, "\t\tUse 0.0.0.0 to listen on all interfaces,\n");
	dprintf(2, "\t\tUse 127.0.0.1 to listen only on localhost\n");
	dprintf(2, "\t\t[env: ADDRESS=%s]\n", envstr("ADDRESS"));

	dprintf(2, "\t-p, --port PORT\n");
	dprintf(2, "\t\tPort number to listen on.\n");
	dprintf(2, "\t\tIf omitted will bind to any free port\n");
	dprintf(2, "\t\t[env: PORT=%s]\n", envstr("PORT"));

	dprintf(2, "\t-t, --trim-newlines[=BOOL]\n");
	dprintf(2, "\t\tRemove trailing newlines (assumes ASCII).\n");
	dprintf(2, "\t\tDon't use this for binary data. \n");
	dprintf(2, "\t\tSets to true if `BOOL` is omitted.\n");
	dprintf(2, "\t[default: false]\n");
	dprintf(2, "\t[env: TRIM_NEWLINES=%s]\n", envstr("TRIM_NEWLINES"));

	return rc;
}

void invalid_input(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vdprintf(2, format, args);
	dprintf(2, "\n");
	va_end(args);
	usage("main", 1);
}

static int parse_int(char *input, int *res)
{
	char *endp;
	long out;
	out = strtol(input, &endp, 0);
	if (*endp != 0)
		return 1;
	*res = (int)out;
	return 0;
}

static int cstreq_i(const char *a, const char *b)
{
	for (; *a && *b; a++, b++) {
		if ((*a | 32) == (*b | 32))
			return (*a | 32) - (*b - 32);
	}
	return 0;
}

static int parse_bool(char *input, int *res)
{
	if (!cstreq_i(input, "1") || !cstreq_i(input, "true") || !cstreq_i(input, "yes") ||
	    !cstreq_i(input, "on")) {
		*res = 1;
		return 0;
	}

	if (!cstreq_i(input, "0") || !cstreq_i(input, "false") || !cstreq_i(input, "no") ||
	    !cstreq_i(input, "off")) {
		*res = 0;
		return 0;
	}
	return 1;
}

int main(int argc, char **argv)
{
	static struct option long_options[] = { { "help", no_argument, 0, 'h' },
						{ "address", required_argument, 0, 'a' },
						{ "port", required_argument, 0, 'p' },
						{ "trim-newlines", optional_argument, 0, 't' },
						{ 0, 0, 0, 0 } };

	int c, rc, option_index = 0, port, sock, ep;
	int trim_newlines, trim_newlines_set;

	char *addr = 0, *portstr = 0;
	port = -1;

	while ((c = getopt_long(argc, argv, "ha:p:", long_options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case 'a':
			addr = optarg;
			break;
		case 'p':
			portstr = optarg;
			break;
		case 't':
			trim_newlines_set = 1;
			if (optarg) {
				if (parse_bool(optarg, &trim_newlines))
					invalid_input("invalid boolean format provided: '%s'",
						      optarg);
			}
			break;

		case '?':
			return 1;
		default:
			break;
		}
	}

	addr = addr ?: (getenv("ADDRESS") ?: "0.0.0.0");
	portstr = portstr ?: (getenv("PORT") ?: 0);

	if (portstr) {
		if (parse_int(portstr, &port))
			invalid_input("Invalid port format: '%s'", portstr);
	} else {
		port = 0;
	}

	// setup socks

	dprintf(2, "Listening on: '%s:%d'\n", addr, port);
	return 0;
}
