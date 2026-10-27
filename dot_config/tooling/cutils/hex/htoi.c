#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <endian.h>
#include <stdarg.h>

#include "lib.h"
#include "cute_log.h"

int usage(char *name, int rc)
{
	fprintf(stderr, "Usage: %s [options] <HEX_STRING | - >\n", name);
	fprintf(stderr,
		"Converts a hex-encode byte sequence back to an unsigned integer\n");
	fprintf(stderr, "and outputs that value in decimal to stdout\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-d, --decode\n");
	fprintf(stderr, "\t\tattempt to decode provided input into utf8\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-b --bytes\n");
	fprintf(stderr,
		"\t\twhat byte width to parse to / output as. must be 1 <= w <= 8\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-s, --seperator <STRING>\n");
	fprintf(stderr, "\t\tString that seperates each hex bytes\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-e, --esc <STRING>\n");
	fprintf(stderr, "\t\tString that prefixes each hex byte\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-w, --keep-whitespace\n");
	fprintf(stderr,
		"\t\tIf provided will not remove trailing whitespace on the input string\n");
	fprintf(stderr, "\n");
	return rc;
}

static void invalid_input(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	usage("htoi", 1);
	exit(1);
}

static int next_byte(const char **p, const char *sep, int seplen, int trim_ws,
		     uint8_t *out)
{
	const char *s = *p;

	while (1) {
		if (seplen && strncmp(s, sep, seplen) == 0) {
			s += seplen;
			continue;
		}
		if (trim_ws && isspace((unsigned char)*s)) {
			s++;
			continue;
		}
		break;
	}

	if (!isxdigit((unsigned char)*s))
		return 0;

	unsigned char hi = *s++;

	while (1) {
		if (seplen && strncmp(s, sep, seplen) == 0) {
			s += seplen;
			continue;
		}
		if (trim_ws && isspace((unsigned char)*s)) {
			s++;
			continue;
		}
		break;
	}

	if (!isxdigit((unsigned char)*s))
		invalid_input(
			"Odd number of hex digits or unexpected character");

	unsigned char lo = *s++;

	*out = (hex_dec_tbl[hi] << 4) | hex_dec_tbl[lo];
	*p = s;
	return 1;
}

static int next_v2(const char **p, const char *end, uint8_t *out)
{
	const char *s = *p;

	while (s < end && !isxdigit((unsigned char)*s)) {
		s++;
	}

	if (s >= end || !isxdigit((unsigned char)*s))
		return 0;

	unsigned char hi = *s++;

	if (s >= end || !isxdigit((unsigned char)*s))
		invalid_input("Odd number of hex digits or unexpected character");

	unsigned char lo = *s++;

	*out = (hex_dec_tbl[hi] << 4) | hex_dec_tbl[lo];
	*p = s;
	return 1;
}

int main(int argc, char **argv)
{
	static struct option long_opts[] = {
		{ "help", no_argument, 0, 'h' },
		{ "bytes", required_argument, 0, 'b' },
		{ "seperator", required_argument, 0, 's' },
		{ "esc", required_argument, 0, 'e' },

		{ "little-endian", no_argument, 0, 'l' },
		{ "keep-whitespace", no_argument, 0, 'w' },
		{ "nl", no_argument, 0, 'n' },
		{ 0, 0, 0, 0 }
	};

	int width = 8;
	int le = 0;
	int trim = 1;
	int print_nl = 0;
	char *sep = "";
	int seplen = 0;

	int c, idx;
	char *endptr;
	char *esc = "";
	int esclen = 0;

	while ((c = getopt_long(argc, argv, "hb:s:lwne:", long_opts, &idx)) !=
	       -1) {
		switch (c) {
		case 'h':
			return usage(argv[0], 0);
		case 'b': {
			long v = strtol(optarg, &endptr, 10);
			if (errno || *endptr || v < 1 || v > 8)
				cute_die(1, errno, "width must be 1-8");
			width = (int)v;
			break;
		}
		case 's':
			sep = optarg ?: "";
			seplen = strlen(sep);
			break;
		case 'e':
			esc = optarg;
			if (!esc)
				cute_die(1, 0, "missing 'esc' string");
			esclen = strlen(esc);
			break;

		case 'l':
			le = 1;
			break;
		case 'w':
			trim = 0;
			break;
		case 'n':
			print_nl = 1;
			break;
		case '?':
			return usage(argv[0], 1);
		default:
			break;
		}
	}

	char ibuf[512];
	const char *istr;
	if (optind < argc) {
		istr = argv[optind];
	} else {
		if (!fgets(ibuf, sizeof ibuf, stdin)) {
			if (ferror(stdin))
				perror("htoi: stdin");
			return 1;
		}
		if (trim) {
			char *nl = strchr(ibuf, '\n');
			if (nl)
				*nl = '\0';
		}
		istr = ibuf;
	}

	if (trim) {
		while (isspace((unsigned char)*istr))
			++istr;
		const char *end = istr + strlen(istr);
		while (end > istr && isspace((unsigned char)*(end - 1)))
			--end;
		char *tmp = strndup(istr, end - istr);
		istr = tmp;
	}

	uint8_t bytes[8] = { 0 };
	int nbytes = 0;
	const char *p = istr;

	while (*p) {
		uint8_t b;
		if (!next_byte(&p, sep, seplen, trim, &b))
			break;
		if (nbytes >= 8)
			invalid_input("Input longer than 8 bytes");
		bytes[nbytes++] = b;
	}

	if (nbytes == 0)
		invalid_input("No hex digits found in input");
	if (nbytes > width)
		invalid_input("Input has %d bytes but --bytes=%d", nbytes,
			      width);

	uint64_t value = 0;
	if (le) {
		for (int i = 0; i < nbytes; i++)
			value |= (uint64_t)bytes[i] << (8 * i);
	} else {
		for (int i = 0; i < nbytes; i++) {
			value <<= 8;
			value |= bytes[i];
		}
	}

	printf("%" PRIu64, value);
	if (print_nl)
		putchar('\n');

	return 0;
}
