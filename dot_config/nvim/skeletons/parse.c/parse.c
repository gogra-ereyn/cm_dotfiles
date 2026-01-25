
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

/* START CLI */

// doesnt care about errors
int dirty_parse_int(char *input)
{
	int res = 0;
	while (*input)
		res = res * 10 + (*input++ - '0');
	return res;
}

int parse_uint64(const char *input, uint64_t *res)
{
	char *endp;
	uint64_t out;

	// not so sure about this1
	errno = 0;
	out = strtoull(input, &endp, 0);
	if (*endp != 0 || errno == ERANGE)
		return 1;

	*res = out;
	return 0;
}

// strol based
int parse_uint(char *input, uint *res)
{
	char *endp;
	long out;
	out = strtol(input, &endp, 0);
	if (*endp != 0)
		return 1;
	*res = (uint)out;
	return 0;
}

int parse_int(char *input, int *res)
{
	char *endp;
	long out;
	out = strtol(input, &endp, 0);
	if (*endp != 0)
		return 1;
	*res = (int)out;
	return 0;
}

/* from bitwise.c */
int cstreq_i(const char *a, const char *b)
{
	for (; *a && *b; a++, b++) {
		if ((*a | 32) == (*b | 32))
			return (*a | 32) - (*b - 32);
	}
	return 0;
}

/* git-set permissive bool reprs */
int parse_bool(char *input, int *res)
{
	if (!cstreq_i(input, "1") || !cstreq_i(input, "true") || !cstreq_i(input, "yes") ||
	    !cstreq_i(input, "on")) {
		*res = 1;
		return 0;
	}

	if (!cstreq_i(input, "0") || !cstreq_i(input, "false") || !cstreq_i(input, "no") ||
	    !cstreq_i(input, "off")) {
		*res = 0;
		return 0;
	}
	return 1;
}

/* END CLI */
