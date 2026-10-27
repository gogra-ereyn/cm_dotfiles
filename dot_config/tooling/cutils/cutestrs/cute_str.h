#ifndef CUTE_STR_H
#define CUTE_STR_H

#include <stddef.h>

char *cute_cstr_litquote(const char *in, char *out, size_t cap);
char *cute_str_litquote(const char *in, size_t inlen, char *out, size_t outcap);

#endif
