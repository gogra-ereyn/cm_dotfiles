#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>


#define MAX 100000

void parray(int *input, int count)
{
	fprintf(stderr, "[parray: %d] [", count);
	int i = 0;

	for (; i < count; ++i) {
		fprintf(stderr, "%d", input[i]);
		if (i != count-1)
			fprintf(stderr, ", ");
	}
	fprintf(stderr, "]\n");
}



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

int solve2(int *nums, int count)
{
	int i;
	char buf[MAX]={0};

	int *ptr=nums;

	for (; ptr < nums+count; ++ptr) {
		if (*ptr > 0 && *ptr <= MAX ) {
			buf[(*ptr)-1]=1;
		}
	}

	// now iter over
	for (i=0; i < MAX; ++i) {
		if (buf[i] == 0) {
			fprintf(stderr, "Found absent positive integer at idx=%d for num=%d\n", i, i+1);
			return i+1;
		}
	}

	fprintf(stderr, "All %d nums present, next (%d) must be missing\n", MAX, MAX+1);
	return MAX+1;
}

int solve(int *nums, int count, int *ans)
{
	*ans=-1;
	int i;
	char buf[MAX]={0};

	int *ptr=nums;
	for (; ptr < nums+count; ++ptr) {
		if (*ptr > 0 && *ptr < MAX ) {
			buf[(*ptr)-1]=1;
		}
	}

	// now iter over
	for (i=0; i < MAX; ++i) {
		if (buf[i] == 0) {
			fprintf(stderr, "Found absent positive integer at idx=%d for num=%d\n", i, i+1);
			*ans=i+1;
			return 0;
		}
	}

	fprintf(stderr, "All %d nums present, next (%d) must be missing\n", MAX, MAX+1);
	*ans=MAX+1;
	return 0;
}


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
	int infd=0;
	while ((c = getopt_long(argc, argv, "hai:", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case 'a':
			append = 1;
			break;
		case 'i':
			if ((infd=open(optarg, O_RDONLY)) < 0) {
				fprintf(stderr, "Failed to open file '%s': %s\n",  optarg, strerror(errno));
				return 1;
			}
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	fprintf(stderr, "Will read input nums from fd=%d\n", infd);

	// scanf-based: requires FILE*
	int *nums;
	nums=calloc(MAX, sizeof(*nums));
	if (!nums) {
		fprintf(stderr, "Failed to calloc: %s\n", strerror(errno));
		return 1;
	}


	FILE *instream;
	instream=fdopen(infd, "r");
	if (!instream) {
		fprintf(stderr, "Failed to fdopen input fd: %s\n", strerror(errno));
		return 1;
	}


	int i=0;
	ssize_t read=0;
	while (i<MAX && fscanf(instream, "%d", &nums[i])==1)
		++i;

	fclose(instream);
	// end

	fprintf(stderr, "Have read %d nums from input\n", i);

	int ans;
	ans=solve2(nums, i);

	fprintf(stderr, "Found first missing=%d\n", ans);
	printf("%d\n", ans);

	//
	// so we have taken flags.
	// now we simply scanf the rest
	return 0;
}
