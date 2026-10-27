#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFSZ 64

int usage(char *name, int rc)
{
	dprintf(2, "Usage: %s [options] [NUMBER]\n", name);
	dprintf(2, "Print the next power of 2 >= NUMBER\n\n");
	dprintf(2, "Options:\n");
	dprintf(2, "\t-h, --help\n");
	dprintf(2, "\t\tShow this help message\n");
	dprintf(2, "\n");
	dprintf(2, "\t-e, --exponent\n");
	dprintf(2, "\t\tPrint the exponent instead of the value\n");
	dprintf(2, "\n");
	return rc;
}

int parse_uint64(const char *input, uint64_t *res)
{
	char *endp;
	uint64_t out;
	errno = 0;
	out = strtoull(input, &endp, 0);
	if (*endp != 0 || endp == input || errno == ERANGE)
		return 1;
	*res = out;
	return 0;
}

int read_uint64(uint64_t *res)
{
	char buf[BUFSZ];
	ssize_t n = read(0, buf, sizeof(buf) - 1);
	if (n <= 0)
		return 1;
	while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == ' '))
		--n;
	buf[n] = 0;
	return parse_uint64(buf, res);
}

/* next power of 2 >= n */
uint64_t p2above(uint64_t n)
{
	int bits;

	if (n <= 1)
		return n;

	bits = 64 - __builtin_clzll(n - 1);
	if (bits >= 64)
		return 0; /* overflow */
	return 1ULL << bits;
}

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "exponent", no_argument, 0, 'e' },
		{ 0, 0, 0, 0 }
	};

	int option_index = 0;
	int c;
	int exponent = 0;
	uint64_t n, result;

	while ((c = getopt_long(argc, argv, "he", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			return usage(*argv, 0);
		case 'e':
			exponent = 1;
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	if (optind < argc) {
		if (parse_uint64(argv[optind], &n)) {
			dprintf(2, "invalid number: '%s'\n", argv[optind]);
			return 1;
		}
	} else {
		if (read_uint64(&n)) {
			dprintf(2, "no input\n");
			return 1;
		}
	}

	result = p2above(n);

	if (exponent) {
		if (result == 0) {
			dprintf(2, "overflow\n");
			return 1;
		}
		dprintf(1, "%d\n", __builtin_ctzll(result));
	} else {
		dprintf(1, "%" PRIu64 "\n", result);
	}

	return 0;
}
