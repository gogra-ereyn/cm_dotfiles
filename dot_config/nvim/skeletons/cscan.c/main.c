#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
//
// see leet/arrays/smallest_positive for inspo/original scanf needs


// fd taht contains ints -> int* without scanf


#define MAX 100000

#define BSIZE 8

static int delim(char c)
{
	return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == ',' ||
	       c == ';';
}

static size_t ring_inc(size_t idx, size_t cap)
{
	return (idx + 1 == cap) ? 0 : idx + 1;
}

static size_t ring_available(size_t r, size_t w, size_t cap)
{
	return (w >= r) ? (cap - (w - r) - 1) : (r - w - 1);
}

static int ring_write(int fd, char *buf, size_t cap, size_t *r, size_t *w)
{
	size_t space = ring_available(*r, *w, cap);
	if (space == 0)
		return 0;
	size_t first = (*w >= *r) ? (cap - *w) : (*r - *w - 1);
	if (first > space)
		first = space;
	ssize_t n = read(fd, buf + *w, first);
	if (n < 0)
		return -1;
	if (n == 0)
		return 1;
	*w = (*w + (size_t)n) % cap;
	return 0;
}

static int ring_parse_next_int(char *buf, size_t cap, size_t *r, size_t w,
			       int *value)
{
	size_t pos = *r;
	while (pos != w && delim(buf[pos]))
		pos = ring_inc(pos, cap);
	if (pos == w)
		return 1;
	int sign = 1;
	char c = buf[pos];
	if (c == '+' || c == '-') {
		if (c == '-')
			sign = -1;
		pos = ring_inc(pos, cap);
		if (pos == w)
			return 1;
		c = buf[pos];
	}
	if (c < '0' || c > '9')
		return -1;
	long v = 0;
	for (;;) {
		v = v * 10 + (c - '0');
		pos = ring_inc(pos, cap);
		if (pos == w)
			return 1;
		c = buf[pos];
		if (c < '0' || c > '9')
			break;
	}
	*value = (int)(sign * v);
	*r = pos;
	return 0;
}

static int ring_parse_ints(char *buf, size_t cap, size_t *r, size_t *w,
			   int *out, size_t out_cap, size_t *nums_written)
{
	size_t n = 0;
	while (n < out_cap) {
		int v;
		int rc = ring_parse_next_int(buf, cap, r, *w, &v);
		if (rc == 0)
			out[n++] = v;
		else if (rc == 1)
			break;
		else
			return rc;
	}
	if (nums_written)
		*nums_written = n;
	return 0;
}


int main(void)
{
	enum { CAP = 4096, NOUT = 1024 };
	char buf[CAP];
	int nums[NOUT];
	size_t r = 0, w = 0;

	for (;;) {

		if (ring_write(STDIN_FILENO, buf, CAP, &r, &w) != 0)
			break;

		size_t n = 0;
		if (ring_parse_ints(buf, CAP, &r, &w, nums, NOUT, &n) != 0)
			break;

		for (size_t i = 0; i < n; ++i)
			printf("%d\n", nums[i]);
	}
	return 0;
}
