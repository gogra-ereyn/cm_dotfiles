#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <endian.h>
#include "lib.h"
#include "cute_log.h"

static inline void print_encoded_bytes(const uint8_t *bytes, int width,
				const char *sep, int seplen, const char *esc, int esclen)
{
	for (int i = 0; i < width; i++) {

		if (esclen)
			fwrite(esc, 1, esclen, stdout);

		putchar(hex_enc_tbl[bytes[i] >> 4]);
		putchar(hex_enc_tbl[bytes[i] & 15]);

		if (seplen && i != width - 1)
			fwrite(sep, 1, seplen, stdout);

	}
}

static void encode(uint64_t v, int le, int width, const char *sep, int seplen, const char *esc, int esclen)
{
	uint8_t b[8];

	if (le) {
		for (int i = 0; i < 8; i++) {
			b[i] = (v >> (i * 8)) & 0xFF;
		}
		print_encoded_bytes(b, width, sep, seplen, esc, esclen);
	} else {
		for (int i = 0; i < 8; i++) {
			b[7 - i] = (v >> (i * 8)) & 0xFF;
		}
		print_encoded_bytes(b + (8 - width), width, sep, seplen, esc, esclen);
	}
}

static int get_min_width(uint64_t n, int le)
{
	if (n == 0)
		return 1;

	if (le) {
		int width = 8;
		uint64_t mask = 0xFF00000000000000ULL;
		while (width > 1 && !(n & mask)) {
			mask >>= 8;
			width--;
		}
		return width;
	} else {
		int width = 1;
		while (n > (1ULL << (width * 8)) - 1) {
			width++;
		}
		return width;
	}
}

int usage(char *name, int rc)
{
	fprintf(stderr, "Usage: %s [options]\n", name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-b --bytes\n");
	fprintf(stderr,
		"\t\twhat byte width to parse to / output as. must be 1 <= w <= 8\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-s, --seperator <STRING>\n");
	fprintf(stderr, "\t\tString to delimit encoded hex bytes\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-e, --esc <STRING>\n");
	fprintf(stderr, "\t\tString with which to prefix each hex byte\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-w, --keep-whitespace\n");
	fprintf(stderr,
		"\t\tIf provided will not remove trailing whitespace on the input string\n");
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
	usage("hex", 1);
}

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "minw", no_argument, 0, 'z' },
		{ "bytes", required_argument, 0, 'b' },
		{ "seperator", required_argument, 0, 's' },
		{ "esc", required_argument, 0, 'e' },
		{ "keep-whitespace", no_argument, 0, 'w' },
		{ "little-endian", no_argument, 0, 'l' },
		{ "nl", no_argument, 0, 'n' },
		{ 0, 0, 0, 0 },
	};

	int le = 0;
	int option_index = 0;
	int c;
	int print_newlin = 0;
	int uppercase = 0;
	char *sep = "";
	int seplen = 0;
	int trim = 1;
	int width = 8;
	char *ep;
	long val;
	int minw=0;
	char *esc="";
	int esclen=0;
	while ((c = getopt_long(argc, argv, "hs:lwnzb:e:", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			return usage(*argv, 0);
		case 'b':
			val = strtol(optarg, &ep, 10);
			if (errno || *ep || val < 1 || val > 8)
				cute_die(1, errno, "Error: width must be between 1 and 8");
			width = val;
			break;
		case 'w':
			trim = 0;
			break;
		case 'n':
			print_newlin = 1;
			break;
		case 'l':
			le = 1;
			break;
		case 'z':
			minw = 1;
			break;
		case 's':
			sep = optarg;
			if (!sep)
				cute_die(1, 0, "missing 'sep' string");
			seplen = strlen(sep);
			break;
		case 'e':
			esc = optarg;
			if (!esc)
				cute_die(1, 0, "missing 'esc' string");
			esclen = strlen(esc);
			break;
		case '?':
			return 1;
		default:
			break;
		};
	}

	char buf[32] = { 0 };
	char *input;

	if (optind < argc) {
		input = argv[optind];
	} else {
		if (!fgets(buf, sizeof(buf), stdin)) {
			if (ferror(stdin)) {
				perror("Error reading stdin");
				return 1;
			}
			return 0;
		}
		char *newline = strchr(buf, '\n');
		if (newline)
			*newline = '\0';
		input = buf;
	}

	char *endptr;
	unsigned long long n = strtoull(input, &endptr, 10);

	if (errno == ERANGE)
		cute_die(1,errno,"Error: integer overflow");

	if (*endptr != '\0' && *endptr != '\n')
		cute_die(1, errno,  "Error: invalid integer '%s'\n");

	if (minw)
		width=get_min_width(n, le);

	encode(n, le, width, sep, seplen, esc, esclen);

	if (print_newlin)
		putchar('\n');

	return 0;
}
