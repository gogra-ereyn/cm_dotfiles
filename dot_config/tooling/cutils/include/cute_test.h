#ifndef CUTE_TEST_H
#define CUTE_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * cute_test.h - minimal test framework
 *
 * two families of macros:
 *   assert_* — abort on failure (hard stop)
 *   check_*  — record failure and continue
 *
 * test runner:
 *   TEST(name) { ... }
 *   TEST_MAIN { RUN_TEST(name); ... TEST_REPORT(); }
 *
 * define CUTE_TEST_IMPL in exactly one .c before including this.
 */

#ifdef CUTE_TEST_IMPL
int _cute_test_failures;
int _cute_test_checks;
int _cute_test_passed;
int _cute_test_failed;
#else
extern int _cute_test_failures;
extern int _cute_test_checks;
extern int _cute_test_passed;
extern int _cute_test_failed;
#endif

/* ---- internal helpers ---- */

#define _cute_test_location() \
	fprintf(stderr, "  File: %s, Line: %d\n", __FILE__, __LINE__)

#define _cute_test_message(...)                                      \
	do {                                                         \
		if (sizeof(#__VA_ARGS__) > 2) {                      \
			fprintf(stderr, "  Message: " __VA_ARGS__);  \
			fputc('\n', stderr);                         \
		}                                                    \
	} while (0)

/* ---- assert: abort on failure ---- */

#define assert_ex(expr, ...)                                              \
	do {                                                              \
		if (!(expr)) {                                            \
			fprintf(stderr, "Assertion failed: %s\n", #expr); \
			_cute_test_location();                            \
			_cute_test_message(__VA_ARGS__);                  \
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
			_cute_test_location();                                             \
			_cute_test_message(__VA_ARGS__);                                   \
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
			_cute_test_location();                                            \
			_cute_test_message(__VA_ARGS__);                                  \
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
				_cute_test_location();                                             \
				_cute_test_message(__VA_ARGS__);                                   \
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
						" 0x%02x vs 0x%02x\n",                            \
						_i, _lc, _rc);                                     \
				}                                                                  \
				_cute_test_location();                                             \
				_cute_test_message(__VA_ARGS__);                                   \
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
			_cute_test_location();                                                    \
			_cute_test_message(__VA_ARGS__);                                          \
			abort();                                                                  \
		}                                                                                 \
	} while (0)

/* ---- check: record failure and continue ---- */

#define check_ex(expr, ...)                                           \
	do {                                                          \
		_cute_test_checks++;                                  \
		if (!(expr)) {                                        \
			fprintf(stderr, "Check failed: %s\n", #expr); \
			_cute_test_location();                        \
			_cute_test_message(__VA_ARGS__);              \
			_cute_test_failures++;                        \
		}                                                     \
	} while (0)

#define check_eq(left, right, ...)                                                         \
	do {                                                                               \
		__typeof__(left) _left_val = (left);                                       \
		__typeof__(right) _right_val = (right);                                    \
		_cute_test_checks++;                                                       \
		if (_left_val != _right_val) {                                             \
			fprintf(stderr, "Check failed: %s == %s\n", #left, #right);        \
			fprintf(stderr, "  Left:  %lld (0x%llx)\n", (long long)_left_val,  \
				(unsigned long long)_left_val);                            \
			fprintf(stderr, "  Right: %lld (0x%llx)\n", (long long)_right_val, \
				(unsigned long long)_right_val);                           \
			_cute_test_location();                                             \
			_cute_test_message(__VA_ARGS__);                                   \
			_cute_test_failures++;                                             \
		}                                                                          \
	} while (0)

#define check_ne(left, right, ...)                                                        \
	do {                                                                              \
		__typeof__(left) _left_val = (left);                                      \
		__typeof__(right) _right_val = (right);                                   \
		_cute_test_checks++;                                                      \
		if (_left_val == _right_val) {                                            \
			fprintf(stderr, "Check failed: %s != %s\n", #left, #right);       \
			fprintf(stderr, "  Value: %lld (0x%llx)\n", (long long)_left_val, \
				(unsigned long long)_left_val);                           \
			_cute_test_location();                                            \
			_cute_test_message(__VA_ARGS__);                                  \
			_cute_test_failures++;                                            \
		}                                                                         \
	} while (0)

#define check_streq(left, right, ...)                                                              \
	do {                                                                                       \
		const char *_left_str = (left);                                                    \
		const char *_right_str = (right);                                                  \
		_cute_test_checks++;                                                               \
		if (_left_str == NULL || _right_str == NULL) {                                     \
			if (_left_str != _right_str) {                                             \
				fprintf(stderr, "Check failed: %s == %s\n", #left, #right);        \
				fprintf(stderr, "  Left:  %s\n",                                   \
					_left_str ? _left_str : "(null)");                         \
				fprintf(stderr, "  Right: %s\n",                                   \
					_right_str ? _right_str : "(null)");                       \
				_cute_test_location();                                             \
				_cute_test_message(__VA_ARGS__);                                   \
				_cute_test_failures++;                                             \
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
						" 0x%02x vs 0x%02x\n",                            \
						_i, _lc, _rc);                                     \
				}                                                                  \
				_cute_test_location();                                             \
				_cute_test_message(__VA_ARGS__);                                   \
				_cute_test_failures++;                                             \
			}                                                                          \
		}                                                                                  \
	} while (0)

#define check_memeq(left, right, len, ...)                                                        \
	do {                                                                                      \
		const unsigned char *_lp = (const unsigned char *)(left);                         \
		const unsigned char *_rp = (const unsigned char *)(right);                        \
		size_t _len = (len);                                                              \
		_cute_test_checks++;                                                              \
		if (memcmp(_lp, _rp, _len) != 0) {                                                \
			size_t _i = 0;                                                            \
			while (_i < _len && _lp[_i] == _rp[_i])                                   \
				_i++;                                                             \
			fprintf(stderr, "Check failed: memcmp(%s, %s, %zu)\n", #left, #right,    \
				_len);                                                            \
			fprintf(stderr,                                                           \
				"  First diff at byte %zu:"                                       \
				" 0x%02x vs 0x%02x\n",                                            \
				_i, _lp[_i], _rp[_i]);                                            \
			_cute_test_location();                                                    \
			_cute_test_message(__VA_ARGS__);                                          \
			_cute_test_failures++;                                                    \
		}                                                                                 \
	} while (0)

/* ---- test runner ---- */

#define TEST(name) static void test_##name(void)

#define RUN_TEST(name)                                                          \
	do {                                                                    \
		int _before = _cute_test_failures;                              \
		fprintf(stderr, "[ RUN  ] %s\n", #name);                        \
		test_##name();                                                  \
		if (_cute_test_failures == _before) {                           \
			fprintf(stderr, "[ PASS ] %s\n", #name);                \
			_cute_test_passed++;                                    \
		} else {                                                        \
			fprintf(stderr, "[ FAIL ] %s (%d failure%s)\n", #name,  \
				_cute_test_failures - _before,                  \
				(_cute_test_failures - _before) == 1 ? "" : "s"); \
			_cute_test_failed++;                                    \
		}                                                               \
	} while (0)

#define TEST_MAIN int main(void)

#define TEST_REPORT()                                                                  \
	do {                                                                           \
		fprintf(stderr, "\n──────────────────────────────\n");                 \
		fprintf(stderr, "%d passed, %d failed (%d checks)\n", _cute_test_passed, \
			_cute_test_failed, _cute_test_checks);                        \
		return _cute_test_failed ? 1 : 0;                                      \
	} while (0)

/* backwards compat with cute_assert.h */
#define cute_assert    assert_ex
#define cute_assert_eq assert_eq

#endif /* CUTE_TEST_H */
