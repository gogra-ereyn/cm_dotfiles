#include <unistd.h>
#include "cute_str.h"

#define BUFSZ 4096

static size_t slen(const char *s)
{
	size_t i;
	for (i = 0; *s; ++i, ++s)
		;
	return i;
}

int main(int argc, char **argv)
{
	char buf[BUFSZ];
	int i;

	if (argc < 2) {
		char in[BUFSZ];
		ssize_t n = read(0, in, sizeof(in) - 1);
		if (n <= 0)
			return 1;
		if (n > 0 && in[n - 1] == '\n')
			--n;
		cute_str_litquote(in, n, buf, sizeof(buf));
		write(1, buf, slen(buf));
		write(1, "\n", 1);
		return 0;
	}

	for (i = 1; i < argc; ++i) {
		if (i > 1)
			write(1, " ", 1);
		cute_cstr_litquote(argv[i], buf, sizeof(buf));
		write(1, buf, slen(buf));
	}
	write(1, "\n", 1);
	return 0;
}
