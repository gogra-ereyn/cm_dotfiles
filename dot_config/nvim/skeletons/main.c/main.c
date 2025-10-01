#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// listen.c for these
int setup_listening_socket(int port);
int setup_listening_socket_randport(int *port);

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

char *envstr(char *name)
{
	char *value;
	value = getenv(name);
	return value ? value : "";
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
	fprintf(stderr, "Usage: %s [options]\n", name);
	fprintf(stderr, "Options:\n");

	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "\t-a, --address <ADDRESS>\n");
	fprintf(stderr, "\t\tInterface address to bind to.\n");
	fprintf(stderr, "\t\tUse 0.0.0.0 to listen on all interfaces,\n");
	fprintf(stderr, "\t\tUse 127.0.0.1 to listen only on localhost\n");
	fprintf(stderr, "\t\t[env: ADDRESS=%s]\n", envstr("ADDRESS"));

	fprintf(stderr, "\t-p, --port PORT\n");
	fprintf(stderr, "\t\tPort number to listen on.\n");
	fprintf(stderr, "\t\tIf omitted will bind to any free port\n");
	fprintf(stderr, "\t\t[env: PORT=%s]\n", envstr("PORT"));

	fprintf(stderr, "\n");
	fprintf(stderr, "Example:\n");
	fprintf(stderr, "\t%s -a 127.0.0.1 -p 3000\n", name);
	fprintf(stderr, "\n");

	return rc;
}

void invalid_input(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
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

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "address", required_argument, 0, 'a' },
		{ "port", required_argument, 0, 'p' },
		{ 0, 0, 0, 0 }
	};

	int c, rc, option_index = 0, port, sock, ep;

	char *addr = 0, *pstr = 0;
	port = -1;

	while ((c = getopt_long(argc, argv, "ha:p:", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case 'a':
			addr = optarg;
			break;
		case 'p':
			rc = parse_int(optarg, &port);
			if (rc)
				invalid_input("Invalid port format '%s'", port);
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	if (!addr) {
		addr = getenv("ADDR");
		if (!addr)
			addr = "0.0.0.0";
	}

	if (port < 0) {
		pstr = getenv("PORT");
		if (!pstr) {
			port = 0;
		} else {
			rc = parse_int(pstr, &port);
			if (rc)
				invalid_input(
					"Invalid env var provided for PORT: '%s'",
					pstr);
		}
	}

	if (!port) {
		sock = setup_listening_socket_randport(&port);
		if (sock == -1)
			t_error(1, errno, "Failed to bind to port 0");
	} else {
		sock = setup_listening_socket(port);
		if (sock == -1)
			t_error(1, errno, "Failed to bind to port '%d'", port);
	}

	dprintf(2, "Listening on: '%s:%d'\n", addr, port);
	return 0;
}
