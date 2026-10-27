#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include "lib.h"

#define CUTE_LOG_LEVEL CUTE_LOG_ERROR
#include "cute_log.h"

static inline size_t hex_encoded_size(size_t n, size_t seplen, size_t esclen)
{
	return n ? n * (2 + esclen) + (n - 1) * seplen : 0;
}

static inline size_t hex_encode(const char *in, size_t n, char *out)
{
	for (size_t i = 0; i < n; i++) {
		char b = in[i];
		*out++ = hex_enc_tbl[b >> 4];
		*out++ = hex_enc_tbl[b & 15];
	}
	return n << 1;
}

static inline char to_alpha_upper(char c, int upper)
{
	if (upper && (c >= 'a' && c <= 'z')) {
		return c & ~32;
	} else {
		return c;
	}
}

static inline size_t hex_encode_delimited(const char *in, size_t n,
					  const char *sep, int seplen,
					  const char *esc, int esclen,
					  char *out, int upper)
{
	size_t i;
	for (i = 0; i < n; i++) {
		if (esclen) {
			memcpy(out, esc, esclen);
			out += esclen;
		}
		unsigned char b = in[i];

		*out++ = to_alpha_upper(hex_enc_tbl[b >> 4], upper);
		*out++ = to_alpha_upper(hex_enc_tbl[b & 15], upper);

		if (upper) {
		}

		if (i != n - 1 && seplen) {
			memcpy(out, sep, seplen);
			out += seplen;
		}
	}
	return n << 1;
}

static inline int is_valid_hex_length(unsigned input_len, int sep_len,
				      int esc_len)
{
	if (input_len == 0)
		return 0;

	if (sep_len == 0 && esc_len == 0)
		return input_len % 2 == 0;

	int pattern_len = esc_len + 2 + sep_len;
	if (pattern_len <= sep_len)
		return 0;

	int adjusted_len = input_len + sep_len;
	return adjusted_len % pattern_len == 0 && adjusted_len > 0;
}

static inline ssize_t hex_decode(const char *in, unsigned n, unsigned stride,
				 unsigned esclen, char *out)
{
	char *st = out;

	unsigned skip = stride + esclen;
	unsigned step = 2 + skip;

	if (!is_valid_hex_length(n, stride, esclen))
		cute_die(1, 0, "Invalid hex sequence: hex bytes must be even");

	for (size_t i = esclen; i < n; i += 2 + skip) {
		char hi = hex_dec_tbl[(char)in[i]],
		     lo = hex_dec_tbl[(char)in[i + 1]];

		if ((hi | lo) == 0xFF)
			return -1;

		*out++ = (hi << 4) | lo;
	}

	return out - st;
}

int usage(char *name, int rc)
{
	fprintf(stderr, "Usage: %s [options]\n", name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-d, --decode\n");
	fprintf(stderr, "\t\tattempt to decode provided input into utf8\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-u, --uppercase\n");
	fprintf(stderr,
		"\t\tWhen encoding, will use upper rather than lowercase hex characters\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-s, --seperator <STRING>\n");
	fprintf(stderr, "\t\tString to delimit encoded hex bytes\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-e, --esc <STRING>\n");
	fprintf(stderr, "\t\tString to prefix each encoded hex byte\n");
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
		{ "decode", no_argument, 0, 'd' },
		{ "seperator", required_argument, 0, 's' },
		{ "esc", required_argument, 0, 'e' },
		{ "keep-whitespace", no_argument, 0, 'w' },
		{ "uppercase", no_argument, 0, 'u' },
		{ "nl", no_argument, 0, 'n' },
		{ 0, 0, 0, 0 },
	};

	int option_index = 0;
	int c;
	int decode = 0;
	int print_newlin = 0;
	int uppercase = 0;

	char *sep = "";
	int seplen = 0;

	char *esc = "";
	int esclen = 0;

	int trim = 1;
	while ((c = getopt_long(argc, argv, "hds:lwne:u", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			return usage(*argv, 0);
		case 'd':
			decode = 1;
			break;
		case 'w':
			trim = 0;
			break;
		case 'n':
			print_newlin = 1;
			break;
		case 'u':
			uppercase = 1;
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

	char *input;
	if (optind == argc) {
		int bsize = 1024;
		input = calloc(bsize, sizeof(input));
		if (!input)
			cute_die(1, errno, "calloc");
		size_t bread = 0;
		size_t total = 0;
		while ((bread = fread(input + total, 1, bsize - total - 1,
				      stdin)) > 0) {
			total += bread;
			if (total > bsize) {
				bsize <<= 1;
				input = realloc(input, bsize);
				if (!input)
					cute_die(1, errno, "realloc");
			}
		}
	} else {
		input = argv[optind];
	}

	int len;
	len = strlen(input);

	cute_info("trim=%d, input is: %s, len=%d, decode=%d, seplen=%d, esclen=%d\n",
	     trim, input, len, decode, seplen, esclen);

	if (trim) {
		int trimlen = len;
		while (trimlen && input[trimlen - 1] == '\n')
			--trimlen;

		input[trimlen] = 0;
		cute_info("Trimmed: %d bytes of trailing whitespace", len - trimlen);
		len = trimlen;
	}

	int outlen;
	char *out;

	if (decode == 0) {
		cute_info("input: %s", input);

		outlen = hex_encoded_size(len, seplen, esclen) + 1;
		out = calloc(outlen, sizeof(*out));

		if (!out)
			cute_die(1, errno, "calloc");

		ssize_t res;
		res = hex_encode_delimited(input, len, sep, seplen, esc, esclen,
					   out, uppercase);

		printf("%s%s", out, print_newlin ? "\n" : "");

		return 0;

	} else {
		char *out;
		out = calloc(len, sizeof(*out));
		if (!out)
			cute_die(1, errno, "calloc");

		ssize_t res;
		res = hex_decode(input, len, seplen, esclen, out);

		fwrite(out, 1, res, stdout);
		if (print_newlin)
			putchar('\n');

		return 0;
	}
}
