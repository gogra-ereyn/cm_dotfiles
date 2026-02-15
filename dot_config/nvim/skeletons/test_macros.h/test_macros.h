#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#include <stdio.h>
#include <stdlib.h>

static int test_pass_count;
static int test_fail_count;

#define TEST_ASSERT(cond)                                                                  \
	do {                                                                               \
		if (!(cond)) {                                                             \
			fprintf(stderr, "  FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
			test_fail_count++;                                                 \
			return;                                                            \
		}                                                                          \
	} while (0)

#define TEST_ASSERT_EQ(a, b)                                                              \
	do {                                                                              \
		if ((a) != (b)) {                                                         \
			fprintf(stderr, "  FAIL: %s:%d: %s == %s (%d != %d)\n", __FILE__, \
				__LINE__, #a, #b, (int)(a), (int)(b));                    \
			test_fail_count++;                                                \
			return;                                                           \
		}                                                                         \
	} while (0)

#define RUN_TEST(fn)                              \
	do {                                      \
		int _before = test_fail_count;    \
		printf("  %-50s", #fn);           \
		fn();                             \
		if (test_fail_count == _before) { \
			test_pass_count++;        \
			printf("PASS\n");         \
		}                                 \
	} while (0)

#define TEST_SUMMARY()                                                                \
	do {                                                                          \
		printf("\n%d passed, %d failed\n", test_pass_count, test_fail_count); \
		return test_fail_count ? EXIT_FAILURE : EXIT_SUCCESS;                 \
	} while (0)

#endif /* TEST_MACROS_H */
