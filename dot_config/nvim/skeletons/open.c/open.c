#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>



/*
 * dumb func for getting files in same dir as source file.
 * nb lazy & havent had to make it work with rel syntax yet.
 * */
static int make_path_relative_to_source(const char *filename, char *out, int out_cap)
{
	const char *p, *d;
	const char *src = __FILE__;
	const char *slash = 0;
	int i, j;
	p = src;
	while (*p) {
		if (*p == '/')
			slash = p;
		++p;
	}
	i = 0;
	if (slash) {
		const char *d = src;
		while (d < slash && i < out_cap - 1) {
			out[i++] = *d++;
		}
		if (i < out_cap - 1)
			out[i++] = '/';
	}
	j = 0;
	while (filename[j] && i < out_cap - 1) {
		out[i++] = filename[j++];
	}
	out[i] = '\0';
	return i;
}

int main(int argc, char **argv)
{
	if (argc == 1)
		return 1;
	char path[PATH_MAX];
	make_path_relative_to_source(argv[1], path, PATH_MAX);
	dprintf(1, "Hallo! will calc relative to: '%s'\n", *argv);
	dprintf(1, "Generated local path: '%s'\n", path);
}
