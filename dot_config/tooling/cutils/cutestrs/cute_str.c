#include "cute_str.h"

static inline size_t cutestrlen(const char *s)
{
	size_t i;
	for (i = 0; *s; ++i, ++s)
		;

	return i;
}

static inline void cutecpy(const char *src, char *dst, size_t n)
{
	int i;
	for (i = 0; i < n; ++i)
		dst[i] = src[i];
}

/*
 * todo - make a public api.
 * return the space required to hold the lit-quoted
 * `src` string.
 * */
size_t cute_lit_size_needed(const char *src, size_t len)
{
	// todo
	return 0;
}

/*
 * literal quoting an input word/string for safe shell/bash usage:
 *	if no internal '
 *	    then surround input in '
 *	else
 *	    escape by
 *		surrounding prefix with '
 *		surrounding suffix with '
 *		escaping ' with \
 *	    e.g.
 *		this'sastring - > 'this'\''sastring'
 * */
char *cute_str_litquote(const char *in, size_t inlen, char *out, size_t outcap)
{
	char *op = out;
	const char *end = in + inlen;
	if (in < end && *in == '\'')
		++in;
	if (in < end && end[-1] == '\'')
		--end;
	*out++ = '\'';
	for (; in < end; ++in) {
		*out++ = *in;
		if (*in == '\'') {
			*out++ = '\\';
			*out++ = '\'';
			*out++ = '\'';
		}
	}
	*out++ = '\'';
	*out = 0;
	return op;
}

char *cute_cstr_litquote(const char *in, char *out, size_t cap)
{
	char *op = out;
	if (*in == '\'')
		++in;
	*out++ = '\'';
	for (; *in; ++in) {
		if (*in == '\'' && !in[1])
			break;
		*out++ = *in;
		if (*in == '\'') {
			*out++ = '\\';
			*out++ = '\'';
			*out++ = '\'';
		}
	}
	*out++ = '\'';
	*out = 0;
	return op;
}
