#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#ifdef DEBUG
#define log(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define log(fmt, ...) ((void)0)
#endif

#define INPUT_MAX 8096

enum {
	RANGE_OK = 0,
	RANGE_ERR_SYNTAX = 1, /* malformed 2....=4" etc.           */
	RANGE_ERR_OOB = 2, /* index outside 0..input_len        */
	RANGE_ERR_ORDER = 3 /* start > end after normalisation   */
};

char *emsg[] = {
	[RANGE_ERR_SYNTAX] = "invalid range syntax",
	[RANGE_ERR_OOB] = "range out of bounds",
	[RANGE_ERR_ORDER] = "start index greater than end index",
};

char *envstr(char *name)
{
	char *value;
	value = getenv(name);
	return value ? value : "";
}

int usage(char *name, int rc)
{
	fprintf(stderr, "Usage: %s [options] <RANGESTR> [INPUT]\n", name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-k, --keep-whitespace\n");
	fprintf(stderr, "\t\tIf set, any/all trailing whitespace on the input string will be kept.\n");
	fprintf(stderr, "\n");
	return rc;
}

void t_error(int status, int errnum, const char *format, ...)
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

void invalid_input(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
	usage("main", 1);
	exit(1);
}

int parse_range(const char *r, int input_len, int *left, int *right)
{
	const char *p = r;

	long v;
	char *ep;
	int start, end;
	int end_inclusive = 0;
	int start_neg = 0, end_neg = 0;

	if (*p == '.') {
		start = 0;
	} else {
		if (*p == '-') {
			start_neg = 1;
			++p;
		}

		v = strtol(p, &ep, 10);
		if (ep == p || errno == ERANGE || v < 0 || v > INT_MAX)
			return RANGE_ERR_SYNTAX;

		start = (int)v;
		if (start_neg) {
			if ((start = input_len - start) < 0)
				return RANGE_ERR_OOB;
		}
		p = ep;
	}

	if (*p++ != '.' || *p++ != '.')
		return RANGE_ERR_SYNTAX;

	if (*p == '=') {
		end_inclusive = 1;
		++p;
	}

	if (*p == 0) {
		end = input_len;
	} else {
		if (*p == '-') {
			end_neg = 1;
			++p;
		}

		v = strtol(p, &ep, 10);
		if (*ep != 0 || ep == p || errno == ERANGE || v < 0 ||
		    v > INT_MAX)
			return RANGE_ERR_SYNTAX;

		end = (int)v + end_inclusive;
		if (end_neg) {
			if ((end = input_len - end) < 0)
				return RANGE_ERR_OOB;
		}
	}

	*left = start;
	*right = end;

	// s a n i t y
	if (start > end)
		return RANGE_ERR_ORDER;

	if (start > input_len || end > input_len)
		return RANGE_ERR_OOB;

	return RANGE_OK;
}

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "keep-whitespace", no_argument, 0, 'k' },
		{ 0, 0, 0, 0 }
	};

	int option_index = 0;
	int c;
	int fd = 0;
	char *s;
	int rc;
	int s_len = 0;
	char *rangestr;
	int trim=1;
	opterr = 0;
	while ((c = getopt_long(argc, argv, "kh", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case 'k':
			trim=0;
			break;
		case '?':
			if ('0' <= argv[optind][1] && '9' >= argv[optind][1])
				goto opts_parsed;
			invalid_input("Unknown option: %s", argv[optind]);
		default:
			break;
		}
	}

opts_parsed:

	if (argc == optind)
		invalid_input("missing range string");

	rangestr = argv[optind];

	if (argc == optind + 1) {
		s = calloc(INPUT_MAX, sizeof(*s));
		if (s_len == 0) {
			size_t bread;
			for (;;) {
				bread = read(0, s + s_len, INPUT_MAX - s_len);

				if (bread == 0)
					break;

				if (bread < 0)
					t_error(1, errno,
						"Error reading from stdin");

				s_len += bread;
			}


		}
	} else {
		s = argv[optind + 1];
		s_len = strlen(s);
	}

	if (trim) {
		while(s_len && s[s_len-1] == '\n')
			--s_len;
		s[s_len]=0;
	}

	log("Have range string: %s, input str of len %d: %s", rangestr, s_len,
	    s);

	int start, end, ec;
	ec = parse_range(rangestr, s_len, &start, &end);

	if (ec != 0) {
		fprintf(stdout, "Error: %s\n", emsg[ec]);
		return ec;
	}

	fwrite(s + start, 1, (size_t)(end - start), stdout);
	putchar('\n');

	return 0;
}
