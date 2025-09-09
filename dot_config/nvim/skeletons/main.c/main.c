#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>

#define DEBUG
#ifdef DEBUG
#define log(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define log(fmt, ...) ((void)0)
#endif


char *envstr(char *name)
{
	char *value;
	value = getenv(name);
	return value ? value : "";
}

int usage(char *name, int rc)
{
	fprintf(stderr, "Usage: %s [options]\n", name);
	fprintf(stderr, "Options:\n");

	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "\t-i, --input FILE\n");
	fprintf(stderr, "\t\tSpecify input file\n");
	fprintf(stderr, "\t\t[default: stdin]\n");
	fprintf(stderr, "\t\t[env: INPUT_FILE=%s]\n", envstr("INPUT_FILE"));
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


int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "input", optional_argument, 0, 'i' },
		{ 0, 0, 0, 0 }
	};

	int option_index = 0;
	int c;
	int fd=0;
	while ((c = getopt_long(argc, argv, "hai:", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case 'i':
			{
				if ((fd = open(optarg, O_RDONLY | O_CLOEXEC)) < 0)
					invalid_input("Failed to open input file: %s", optarg);
			}
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	return 0;
}
