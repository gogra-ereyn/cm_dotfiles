#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

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

typedef enum { ENC_UTF8, ENC_HEX, ENC_BASE64, ENC_UNKNOWN } encoding_t;





int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "append", optional_argument, 0, 'a' },
		{ "input", optional_argument, 0, 'i' },
		{ 0, 0, 0, 0 }
	};

	int option_index = 0;
	int c;
	int append = 0;
	while ((c = getopt_long(argc, argv, "hai:", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case 'e':
			// todo
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	return 0;
}
