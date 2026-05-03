#ifndef MASSERT_H
#define MASSERT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
a currently-exploratory extention of massert.h for dedicated usage in tests.
 * */

/*
 * test framework state — define MASSERT_IMPL in exactly one .c file before
 * including this. everything else gets externs.
 */

#ifdef MASSERT_IMPL
int _massert_failures = 0;
int _massert_checks = 0;
#else
extern int _massert_failures;
extern int _massert_checks;
#endif

#ifndef MASSERT_ARR_MAX_PRINT
#define MASSERT_ARR_MAX_PRINT 16
#endif

#define _massert_location() fprintf(stderr, "  File: %s, Line: %d\n", __FILE__, __LINE__)

#define _massert_message(...)                                       \
	do {                                                        \
		if (sizeof(#__VA_ARGS__) > 2) {                     \
			fprintf(stderr, "  Message: " __VA_ARGS__); \
			fputc('\n', stderr);                        \
		}                                                   \
	} while (0)

#define assert_ex(expr, ...)                                              \
	do {                                                              \
		if (!(expr)) {                                            \
			fprintf(stderr, "Assertion failed: %s\n", #expr); \
			_massert_location();                              \
			_massert_message(__VA_ARGS__);                    \
			abort();                                          \
		}                                                         \
	} while (0)

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
			_massert_location();                                               \
			_massert_message(__VA_ARGS__);                                     \
			abort();                                                           \
		}                                                                          \
	} while (0)

#define assert_ne(left, right, ...)                                                       \
	do {                                                                              \
		__typeof__(left) _left_val = (left);                                      \
		__typeof__(right) _right_val = (right);                                   \
		if (_left_val == _right_val) {                                            \
			fprintf(stderr, "Assertion failed: %s != %s\n", #left, #right);   \
			fprintf(stderr, "  Value: %lld (0x%llx)\n", (long long)_left_val, \
				(unsigned long long)_left_val);                           \
			_massert_location();                                              \
			_massert_message(__VA_ARGS__);                                    \
			abort();                                                          \
		}                                                                         \
	} while (0)

#define assert_streq(left, right, ...)                                                             \
	do {                                                                                       \
		const char *_left_str = (left);                                                    \
		const char *_right_str = (right);                                                  \
		if (_left_str == NULL || _right_str == NULL) {                                     \
			if (_left_str != _right_str) {                                             \
				fprintf(stderr, "Assertion failed: %s == %s\n", #left, #right);    \
				fprintf(stderr, "  Left:  %s\n",                                   \
					_left_str ? _left_str : "(null)");                         \
				fprintf(stderr, "  Right: %s\n",                                   \
					_right_str ? _right_str : "(null)");                       \
				_massert_location();                                               \
				_massert_message(__VA_ARGS__);                                     \
				abort();                                                           \
			}                                                                          \
		} else {                                                                           \
			size_t _left_len = strlen(_left_str);                                      \
			size_t _right_len = strlen(_right_str);                                    \
			if (_left_len != _right_len ||                                             \
			    memcmp(_left_str, _right_str, _left_len) != 0) {                       \
				size_t _i = 0;                                                     \
				fprintf(stderr, "Assertion failed: %s == %s\n", #left, #right);    \
				fprintf(stderr, "  Left:  \"%s\" (len=%zu)\n", _left_str,          \
					_left_len);                                                \
				fprintf(stderr, "  Right: \"%s\" (len=%zu)\n", _right_str,         \
					_right_len);                                               \
				while (_i < _left_len && _i < _right_len &&                        \
				       _left_str[_i] == _right_str[_i])                            \
					_i++;                                                      \
				if (_i < _left_len || _i < _right_len) {                           \
					unsigned _lc =                                             \
						_i < _left_len ? (unsigned char)_left_str[_i] : 0; \
					unsigned _rc = _i < _right_len ?                           \
							       (unsigned char)_right_str[_i] :     \
							       0;                                  \
					fprintf(stderr,                                            \
						"  First diff at index %zu:"                       \
						" 0x%02x vs 0x%02x\n",                             \
						_i, _lc, _rc);                                     \
				}                                                                  \
				_massert_location();                                               \
				_massert_message(__VA_ARGS__);                                     \
				abort();                                                           \
			}                                                                          \
		}                                                                                  \
	} while (0)

#define assert_memeq(left, right, len, ...)                                                       \
	do {                                                                                      \
		const unsigned char *_lp = (const unsigned char *)(left);                         \
		const unsigned char *_rp = (const unsigned char *)(right);                        \
		size_t _len = (len);                                                              \
		if (memcmp(_lp, _rp, _len) != 0) {                                                \
			size_t _i = 0;                                                            \
			while (_i < _len && _lp[_i] == _rp[_i])                                   \
				_i++;                                                             \
			fprintf(stderr, "Assertion failed: memcmp(%s, %s, %zu)\n", #left, #right, \
				_len);                                                            \
			fprintf(stderr,                                                           \
				"  First diff at byte %zu:"                                       \
				" 0x%02x vs 0x%02x\n",                                            \
				_i, _lp[_i], _rp[_i]);                                            \
			_massert_location();                                                      \
			_massert_message(__VA_ARGS__);                                            \
			abort();                                                                  \
		}                                                                                 \
	} while (0)

#define _check_fail()                \
	do {                         \
		_massert_failures++; \
	} while (0)

#define check_ex(expr, ...)                                           \
	do {                                                          \
		_massert_checks++;                                    \
		if (!(expr)) {                                        \
			fprintf(stderr, "Check failed: %s\n", #expr); \
			_massert_location();                          \
			_massert_message(__VA_ARGS__);                \
			_check_fail();                                \
		}                                                     \
	} while (0)

#define check_eq(left, right, ...)                                                         \
	do {                                                                               \
		__typeof__(left) _left_val = (left);                                       \
		__typeof__(right) _right_val = (right);                                    \
		_massert_checks++;                                                         \
		if (_left_val != _right_val) {                                             \
			fprintf(stderr, "Check failed: %s == %s\n", #left, #right);        \
			fprintf(stderr, "  Left:  %lld (0x%llx)\n", (long long)_left_val,  \
				(unsigned long long)_left_val);                            \
			fprintf(stderr, "  Right: %lld (0x%llx)\n", (long long)_right_val, \
				(unsigned long long)_right_val);                           \
			_massert_location();                                               \
			_massert_message(__VA_ARGS__);                                     \
			_check_fail();                                                     \
		}                                                                          \
	} while (0)

#define check_ne(left, right, ...)                                                        \
	do {                                                                              \
		__typeof__(left) _left_val = (left);                                      \
		__typeof__(right) _right_val = (right);                                   \
		_massert_checks++;                                                        \
		if (_left_val == _right_val) {                                            \
			fprintf(stderr, "Check failed: %s != %s\n", #left, #right);       \
			fprintf(stderr, "  Value: %lld (0x%llx)\n", (long long)_left_val, \
				(unsigned long long)_left_val);                           \
			_massert_location();                                              \
			_massert_message(__VA_ARGS__);                                    \
			_check_fail();                                                    \
		}                                                                         \
	} while (0)

#define check_streq(left, right, ...)                                                              \
	do {                                                                                       \
		const char *_left_str = (left);                                                    \
		const char *_right_str = (right);                                                  \
		_massert_checks++;                                                                 \
		if (_left_str == NULL || _right_str == NULL) {                                     \
			if (_left_str != _right_str) {                                             \
				fprintf(stderr, "Check failed: %s == %s\n", #left, #right);        \
				fprintf(stderr, "  Left:  %s\n",                                   \
					_left_str ? _left_str : "(null)");                         \
				fprintf(stderr, "  Right: %s\n",                                   \
					_right_str ? _right_str : "(null)");                       \
				_massert_location();                                               \
				_massert_message(__VA_ARGS__);                                     \
				_check_fail();                                                     \
			}                                                                          \
		} else {                                                                           \
			size_t _left_len = strlen(_left_str);                                      \
			size_t _right_len = strlen(_right_str);                                    \
			if (_left_len != _right_len ||                                             \
			    memcmp(_left_str, _right_str, _left_len) != 0) {                       \
				size_t _i = 0;                                                     \
				fprintf(stderr, "Check failed: %s == %s\n", #left, #right);        \
				fprintf(stderr, "  Left:  \"%s\" (len=%zu)\n", _left_str,          \
					_left_len);                                                \
				fprintf(stderr, "  Right: \"%s\" (len=%zu)\n", _right_str,         \
					_right_len);                                               \
				while (_i < _left_len && _i < _right_len &&                        \
				       _left_str[_i] == _right_str[_i])                            \
					_i++;                                                      \
				if (_i < _left_len || _i < _right_len) {                           \
					unsigned _lc =                                             \
						_i < _left_len ? (unsigned char)_left_str[_i] : 0; \
					unsigned _rc = _i < _right_len ?                           \
							       (unsigned char)_right_str[_i] :     \
							       0;                                  \
					fprintf(stderr,                                            \
						"  First diff at index %zu:"                       \
						" 0x%02x vs 0x%02x\n",                             \
						_i, _lc, _rc);                                     \
				}                                                                  \
				_massert_location();                                               \
				_massert_message(__VA_ARGS__);                                     \
				_check_fail();                                                     \
			}                                                                          \
		}                                                                                  \
	} while (0)

#define assert_eq_f64(left, right, ...)                                             \
	do {                                                                       \
		double _left_val = (left);                                         \
		double _right_val = (right);                                       \
		if (_left_val != _right_val) {                                     \
			fprintf(stderr, "Assertion failed: %s == %s\n",            \
				#left, #right);                                    \
			fprintf(stderr, "  Left:  %.17g\n", _left_val);           \
			fprintf(stderr, "  Right: %.17g\n", _right_val);          \
			_massert_location();                                       \
			_massert_message(__VA_ARGS__);                             \
			abort();                                                   \
		}                                                                  \
	} while (0)

#define check_eq_f64(left, right, ...)                                             \
	do {                                                                       \
		double _left_val = (left);                                         \
		double _right_val = (right);                                       \
		_massert_checks++;                                                 \
		if (_left_val != _right_val) {                                     \
			fprintf(stderr, "Check failed: %s == %s\n",                \
				#left, #right);                                    \
			fprintf(stderr, "  Left:  %.17g\n", _left_val);           \
			fprintf(stderr, "  Right: %.17g\n", _right_val);          \
			_massert_location();                                       \
			_massert_message(__VA_ARGS__);                             \
			_check_fail();                                             \
		}                                                                  \
	} while (0)

#define _MASSERT_ARR_PRINT_I32(lp, rp, len, mismatches)                                 \
	do {                                                                               \
		int _p = 0;                                                                \
		fprintf(stderr, "  idx  got              expected\n");                      \
		for (_i = 0; _i < len; _i++) {                                             \
			if ((lp)[_i] != (rp)[_i]) {                                        \
				if (MASSERT_ARR_MAX_PRINT && _p >= MASSERT_ARR_MAX_PRINT) { \
					fprintf(stderr, "  ... and %d more\n",             \
						mismatches - _p);                          \
					break;                                             \
				}                                                          \
				fprintf(stderr, "  [%d]  %-16d%d\n",                       \
					_i, (lp)[_i], (rp)[_i]);                          \
				_p++;                                                      \
			}                                                                  \
		}                                                                          \
	} while (0)

#define assert_arreq_i32(left, right, len, ...)                                            \
	do {                                                                                  \
		int _len = (len);                                                             \
		int _i, _mismatches = 0;                                                      \
		int _lbuf[_len], _rbuf[_len];                                                 \
		for (_i = 0; _i < _len; _i++) { _lbuf[_i] = (left)[_i]; _rbuf[_i] = (right)[_i]; } \
		for (_i = 0; _i < _len; _i++)                                                 \
			if (_lbuf[_i] != _rbuf[_i]) _mismatches++;                            \
		if (_mismatches) {                                                             \
			fprintf(stderr, "Assertion failed: %s == %s (%d elems)\n",             \
				#left, #right, _len);                                          \
			_MASSERT_ARR_PRINT_I32(_lbuf, _rbuf, _len, _mismatches);              \
			fprintf(stderr, "  %d/%d mismatches\n", _mismatches, _len);            \
			_massert_location();                                                   \
			_massert_message(__VA_ARGS__);                                         \
			abort();                                                               \
		}                                                                              \
	} while (0)

#define _MASSERT_ARR_PRINT_I64(lp, rp, len, mismatches)                                 \
	do {                                                                               \
		int _p = 0;                                                                \
		fprintf(stderr, "  idx  got              expected\n");                      \
		for (_i = 0; _i < len; _i++) {                                             \
			if ((lp)[_i] != (rp)[_i]) {                                        \
				if (MASSERT_ARR_MAX_PRINT && _p >= MASSERT_ARR_MAX_PRINT) { \
					fprintf(stderr, "  ... and %d more\n",             \
						mismatches - _p);                          \
					break;                                             \
				}                                                          \
				fprintf(stderr, "  [%d]  %-16lld%lld\n",                   \
					_i, (lp)[_i], (rp)[_i]);                          \
				_p++;                                                      \
			}                                                                  \
		}                                                                          \
	} while (0)

#define _MASSERT_ARR_PRINT_U64(lp, rp, len, mismatches)                                \
	do {                                                                               \
		int _p = 0;                                                                \
		fprintf(stderr, "  idx  got              expected\n");                      \
		for (_i = 0; _i < len; _i++) {                                             \
			if ((lp)[_i] != (rp)[_i]) {                                        \
				if (MASSERT_ARR_MAX_PRINT && _p >= MASSERT_ARR_MAX_PRINT) { \
					fprintf(stderr, "  ... and %d more\n",             \
						mismatches - _p);                          \
					break;                                             \
				}                                                          \
				fprintf(stderr, "  [%d]  %-16llu%llu\n",                   \
					_i, (lp)[_i], (rp)[_i]);                          \
				_p++;                                                      \
			}                                                                  \
		}                                                                          \
	} while (0)

#define _MASSERT_ARR_PRINT_F64(lp, rp, len, mismatches)                                \
	do {                                                                               \
		int _p = 0;                                                                \
		fprintf(stderr, "  idx  got              expected\n");                      \
		for (_i = 0; _i < len; _i++) {                                             \
			if ((lp)[_i] != (rp)[_i]) {                                        \
				if (MASSERT_ARR_MAX_PRINT && _p >= MASSERT_ARR_MAX_PRINT) { \
					fprintf(stderr, "  ... and %d more\n",             \
						mismatches - _p);                          \
					break;                                             \
				}                                                          \
				fprintf(stderr, "  [%d]  %-16g%g\n",                       \
					_i, (lp)[_i], (rp)[_i]);                          \
				_p++;                                                      \
			}                                                                  \
		}                                                                          \
	} while (0)

#define assert_arreq_i64(left, right, len, ...)                                            \
	do {                                                                                  \
		const long long *_lp;                                                         \
		const long long *_rp;                                                         \
		int _len = (len);                                                             \
		int _i, _mismatches = 0;                                                      \
		long long _lbuf[_len], _rbuf[_len];                                           \
		for (_i = 0; _i < _len; _i++) { _lbuf[_i] = (left)[_i]; _rbuf[_i] = (right)[_i]; } \
		_lp = _lbuf; _rp = _rbuf;                                                    \
		for (_i = 0; _i < _len; _i++)                                                 \
			if (_lp[_i] != _rp[_i]) _mismatches++;                                \
		if (_mismatches) {                                                             \
			fprintf(stderr, "Assertion failed: %s == %s (%d elems)\n",             \
				#left, #right, _len);                                          \
			_MASSERT_ARR_PRINT_I64(_lp, _rp, _len, _mismatches);                  \
			fprintf(stderr, "  %d/%d mismatches\n", _mismatches, _len);            \
			_massert_location();                                                   \
			_massert_message(__VA_ARGS__);                                         \
			abort();                                                               \
		}                                                                              \
	} while (0)

#define assert_arreq_u64(left, right, len, ...)                                                \
	do {                                                                                  \
		int _len = (len);                                                             \
		int _i, _mismatches = 0;                                                      \
		unsigned long long _lbuf[_len], _rbuf[_len];                                  \
		for (_i = 0; _i < _len; _i++) { _lbuf[_i] = (left)[_i]; _rbuf[_i] = (right)[_i]; } \
		for (_i = 0; _i < _len; _i++)                                                 \
			if (_lbuf[_i] != _rbuf[_i]) _mismatches++;                            \
		if (_mismatches) {                                                             \
			fprintf(stderr, "Assertion failed: %s == %s (%d elems)\n",             \
				#left, #right, _len);                                          \
			_MASSERT_ARR_PRINT_U64(_lbuf, _rbuf, _len, _mismatches);              \
			fprintf(stderr, "  %d/%d mismatches\n", _mismatches, _len);            \
			_massert_location();                                                   \
			_massert_message(__VA_ARGS__);                                         \
			abort();                                                               \
		}                                                                              \
	} while (0)

#define assert_arreq_f64(left, right, len, ...)                                        \
	do {                                                                           \
		const double *_lp = (const double *)(left);                            \
		const double *_rp = (const double *)(right);                           \
		int _len = (len);                                                      \
		int _i, _mismatches = 0;                                               \
		for (_i = 0; _i < _len; _i++)                                          \
			if (_lp[_i] != _rp[_i]) _mismatches++;                         \
		if (_mismatches) {                                                      \
			fprintf(stderr, "Assertion failed: %s == %s (%d elems)\n",      \
				#left, #right, _len);                                   \
			_MASSERT_ARR_PRINT_F64(_lp, _rp, _len, _mismatches);           \
			fprintf(stderr, "  %d/%d mismatches\n", _mismatches, _len);     \
			_massert_location();                                            \
			_massert_message(__VA_ARGS__);                                  \
			abort();                                                        \
		}                                                                       \
	} while (0)

#define check_arreq_i64(left, right, len, ...)                                                \
	do {                                                                                  \
		const long long *_lp;                                                         \
		const long long *_rp;                                                         \
		int _len = (len);                                                             \
		int _i, _mismatches = 0;                                                      \
		long long _lbuf[_len], _rbuf[_len];                                           \
		for (_i = 0; _i < _len; _i++) { _lbuf[_i] = (left)[_i]; _rbuf[_i] = (right)[_i]; } \
		_lp = _lbuf; _rp = _rbuf;                                                    \
		_massert_checks++;                                                            \
		for (_i = 0; _i < _len; _i++)                                                 \
			if (_lp[_i] != _rp[_i]) _mismatches++;                                \
		if (_mismatches) {                                                             \
			fprintf(stderr, "Check failed: %s == %s (%d elems)\n",                \
				#left, #right, _len);                                          \
			_MASSERT_ARR_PRINT_I64(_lp, _rp, _len, _mismatches);                  \
			fprintf(stderr, "  %d/%d mismatches\n", _mismatches, _len);            \
			_massert_location();                                                   \
			_massert_message(__VA_ARGS__);                                         \
			_check_fail();                                                         \
		}                                                                              \
	} while (0)

#define check_arreq_u64(left, right, len, ...)                                                 \
	do {                                                                                  \
		int _len = (len);                                                             \
		int _i, _mismatches = 0;                                                      \
		unsigned long long _lbuf[_len], _rbuf[_len];                                  \
		for (_i = 0; _i < _len; _i++) { _lbuf[_i] = (left)[_i]; _rbuf[_i] = (right)[_i]; } \
		_massert_checks++;                                                            \
		for (_i = 0; _i < _len; _i++)                                                 \
			if (_lbuf[_i] != _rbuf[_i]) _mismatches++;                            \
		if (_mismatches) {                                                             \
			fprintf(stderr, "Check failed: %s == %s (%d elems)\n",                \
				#left, #right, _len);                                          \
			_MASSERT_ARR_PRINT_U64(_lbuf, _rbuf, _len, _mismatches);              \
			fprintf(stderr, "  %d/%d mismatches\n", _mismatches, _len);            \
			_massert_location();                                                   \
			_massert_message(__VA_ARGS__);                                         \
			_check_fail();                                                         \
		}                                                                              \
	} while (0)

#define check_arreq_f64(left, right, len, ...)                                         \
	do {                                                                           \
		const double *_lp = (const double *)(left);                            \
		const double *_rp = (const double *)(right);                           \
		int _len = (len);                                                      \
		int _i, _mismatches = 0;                                               \
		_massert_checks++;                                                     \
		for (_i = 0; _i < _len; _i++)                                          \
			if (_lp[_i] != _rp[_i]) _mismatches++;                         \
		if (_mismatches) {                                                      \
			fprintf(stderr, "Check failed: %s == %s (%d elems)\n",         \
				#left, #right, _len);                                   \
			_MASSERT_ARR_PRINT_F64(_lp, _rp, _len, _mismatches);           \
			fprintf(stderr, "  %d/%d mismatches\n", _mismatches, _len);     \
			_massert_location();                                            \
			_massert_message(__VA_ARGS__);                                  \
			_check_fail();                                                  \
		}                                                                       \
	} while (0)

#define check_memeq(left, right, len, ...)                                                    \
	do {                                                                                  \
		const unsigned char *_lp = (const unsigned char *)(left);                     \
		const unsigned char *_rp = (const unsigned char *)(right);                    \
		size_t _len = (len);                                                          \
		_massert_checks++;                                                            \
		if (memcmp(_lp, _rp, _len) != 0) {                                            \
			size_t _i = 0;                                                        \
			while (_i < _len && _lp[_i] == _rp[_i])                               \
				_i++;                                                         \
			fprintf(stderr, "Check failed: memcmp(%s, %s, %zu)\n", #left, #right, \
				_len);                                                        \
			fprintf(stderr,                                                       \
				"  First diff at byte %zu:"                                   \
				" 0x%02x vs 0x%02x\n",                                        \
				_i, _lp[_i], _rp[_i]);                                        \
			_massert_location();                                                  \
			_massert_message(__VA_ARGS__);                                        \
			_check_fail();                                                        \
		}                                                                             \
	} while (0)

/*  ejemplo: minimal test runner
 *
 * Usage:
 *
 *   #define MASSERT_IMPL
 *   #include "massert.h"
 *
 *   TEST(my_test_name) {
 *       check_eq(1 + 1, 2);
 *       check_streq("hello", "hello");
 *   }
 *
 *   TEST(another_test) {
 *       check_ne(42, 0);
 *   }
 *
 *   TEST_MAIN {
 *       RUN_TEST(my_test_name);
 *       RUN_TEST(another_test);
 *   }
 */

#define TEST(name) static void test_##name(void)

#define RUN_TEST(name)                                                          \
	do {                                                                    \
		int _before = _massert_failures;                                \
		fprintf(stderr, "[ RUN  ] %s\n", #name);                        \
		test_##name();                                                  \
		if (_massert_failures == _before) {                             \
			fprintf(stderr, "[ PASS ] %s\n", #name);                \
			_massert_passed++;                                      \
		} else {                                                        \
			fprintf(stderr, "[ FAIL ] %s (%d failure%s)\n", #name,  \
				_massert_failures - _before,                    \
				(_massert_failures - _before) == 1 ? "" : "s"); \
			_massert_test_failures++;                               \
		}                                                               \
	} while (0)

/*
 * TEST_MAIN: generates main(). follow with a braced block of RUN_TEST() calls.
 * report and exit code are automatic.
 */
#define TEST_MAIN                                                                      \
	static int _massert_passed;                                                    \
	static int _massert_test_failures;                                             \
	static void _run_tests(void);                                                  \
	int main(void) {                                                               \
		_run_tests();                                                          \
		fprintf(stderr, "\n──────────────────────────────\n");                 \
		fprintf(stderr, "%d passed, %d failed (%d checks)\n", _massert_passed, \
			_massert_test_failures, _massert_checks);                      \
		return _massert_test_failures ? 1 : 0;                                 \
	}                                                                              \
	static void _run_tests(void)

#endif /* MASSERT_H */
