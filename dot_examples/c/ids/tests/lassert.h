#ifndef MASSERT_H
#define MASSERT_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define a(expr)                                                             \
	do {                                                                \
		if (!(expr)) {                                              \
			fprintf(stderr,                                     \
				"Assertion failed: %s, file %s, line %d\n", \
				#expr, __FILE__, __LINE__);                 \
			abort();                                            \
		}                                                           \
	} while (0)

#define assert_eq(left, right, ...)                                            \
	do {                                                                   \
		__typeof__(left) _left_val = (left);                           \
		__typeof__(right) _right_val = (right);                        \
		if (_left_val != _right_val) {                                 \
			fprintf(stderr, "Assertion failed: %s == %s\n", #left, \
				#right);                                       \
			fprintf(stderr, "  Left:  %lld (0x%llx)\n",            \
				(long long)_left_val,                          \
				(unsigned long long)_left_val);                \
			fprintf(stderr, "  Right: %lld (0x%llx)\n",            \
				(long long)_right_val,                         \
				(unsigned long long)_right_val);               \
			fprintf(stderr, "  File: %s, Line: %d\n", __FILE__,    \
				__LINE__);                                     \
			if (sizeof(#__VA_ARGS__) > 2) {                        \
				fprintf(stderr, "  Message: " __VA_ARGS__);    \
				fputc('\n', stderr);                           \
			}                                                      \
			abort();                                               \
		}                                                              \
	} while (0)

#define assert_ex(expr, ...)                                                \
	do {                                                                \
		if (!(expr)) {                                              \
			fprintf(stderr, "Assertion failed: %s\n", #expr);   \
			fprintf(stderr, "  File: %s, Line: %d\n", __FILE__, \
				__LINE__);                                  \
			if (sizeof(#__VA_ARGS__) > 2) {                     \
				fprintf(stderr, "  Message: " __VA_ARGS__); \
				fputc('\n', stderr);                        \
			}                                                   \
			abort();                                            \
		}                                                           \
	} while (0)

#endif

