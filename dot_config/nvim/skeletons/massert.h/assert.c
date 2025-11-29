#ifndef MASSERT_H
#define MASSERT_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define assert_eq(left, right, ...)                                                        \
	do {                                                                               \
		__typeof__(left) _left_val = (left);                                       \
		__typeof__(right) _right_val = (right);                                    \
		if (_left_val != _right_val) {                                             \
			fprintf(stderr, "Assertion failed: %s == %s\n", #left, #right);    \
			fprintf(stderr, "  Left:  %lld (0x%llx)\n", (long long)_left_val,  \
				(unsigned long long)_left_val);                            \
			fprintf(stderr, "  Right: %lld (0x%llx)\n", (long long)_right_val, \
				(unsigned long long)_right_val);                           \
			fprintf(stderr, "  File: %s, Line: %d\n", __FILE__, __LINE__);     \
			if (sizeof(#__VA_ARGS__) > 2) {                                    \
				fprintf(stderr, "  Message: " __VA_ARGS__);                \
				fputc('\n', stderr);                                       \
			}                                                                  \
			abort();                                                           \
		}                                                                          \
	} while (0)

#define assert_ex(expr, ...)                                                           \
	do {                                                                           \
		if (!(expr)) {                                                         \
			fprintf(stderr, "Assertion failed: %s\n", #expr);              \
			fprintf(stderr, "  File: %s, Line: %d\n", __FILE__, __LINE__); \
			if (sizeof(#__VA_ARGS__) > 2) {                                \
				fprintf(stderr, "  Message: " __VA_ARGS__);            \
				fputc('\n', stderr);                                   \
			}                                                              \
			abort();                                                       \
		}                                                                      \
	} while (0)

#define assert_streq(left, right, ...)                                                                 \
	do {                                                                                           \
		const char *_left_str;                                                                 \
		const char *_right_str;                                                                \
		size_t _left_len;                                                                      \
		size_t _right_len;                                                                     \
		size_t _i;                                                                             \
		_left_str = (left);                                                                    \
		_right_str = (right);                                                                  \
		if (_left_str == NULL || _right_str == NULL) {                                         \
			if (_left_str != _right_str) {                                                 \
				fprintf(stderr, "Assertion failed: %s == %s\n", #left, #right);        \
				fprintf(stderr, "  Left:  %s\n",                                       \
					_left_str ? _left_str : "(null)");                             \
				fprintf(stderr, "  Right: %s\n",                                       \
					_right_str ? _right_str : "(null)");                           \
				fprintf(stderr, "  File: %s, Line: %d\n", __FILE__, __LINE__);         \
				if (sizeof(#__VA_ARGS__) > 2) {                                        \
					fprintf(stderr, "  Message: " __VA_ARGS__);                    \
					fputc('\n', stderr);                                           \
				}                                                                      \
				abort();                                                               \
			}                                                                              \
		} else {                                                                               \
			_left_len = strlen(_left_str);                                                 \
			_right_len = strlen(_right_str);                                               \
			if (_left_len != _right_len ||                                                 \
			    memcmp(_left_str, _right_str, _left_len) != 0) {                           \
				fprintf(stderr, "Assertion failed: %s == %s\n", #left, #right);        \
				fprintf(stderr, "  Left:  \"%s\" (len=%zu)\n", _left_str,              \
					_left_len);                                                    \
				fprintf(stderr, "  Right: \"%s\" (len=%zu)\n", _right_str,             \
					_right_len);                                                   \
				_i = 0;                                                                \
				while (_i < _left_len && _i < _right_len &&                            \
				       _left_str[_i] == _right_str[_i]) {                              \
					_i++;                                                          \
				}                                                                      \
				if (_i < _left_len || _i < _right_len) {                               \
					unsigned int _lc;                                              \
					unsigned int _rc;                                              \
					_lc = _i < _left_len ? (unsigned char)_left_str[_i] : 0;       \
					_rc = _i < _right_len ? (unsigned char)_right_str[_i] : 0;     \
					fprintf(stderr,                                                \
						"  First difference at index %zu: 0x%02x vs 0x%02x\n", \
						_i, _lc, _rc);                                         \
				}                                                                      \
				fprintf(stderr, "  File: %s, Line: %d\n", __FILE__, __LINE__);         \
				if (sizeof(#__VA_ARGS__) > 2) {                                        \
					fprintf(stderr, "  Message: " __VA_ARGS__);                    \
					fputc('\n', stderr);                                           \
				}                                                                      \
				abort();                                                               \
			}                                                                              \
		}                                                                                      \
	} while (0)

#endif
