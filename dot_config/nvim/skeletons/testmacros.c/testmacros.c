/* giga unpolished, expects tests_passed to be declared in current scope */

#define TEST_START(name)                                 \
	do {                                             \
		test_num++;                              \
		dprintf("Test %d: %s\n", test_num, name); \
	} while (0)

#define ASSERT(cond, msg)                                      \
	do {                                                   \
		if (!(cond)) {                                 \
			dprintf("  FAIL: %s\n", msg);          \
			dprintf("    at line %d\n", __LINE__); \
			tests_failed++;                        \
			return;                                \
		}                                              \
	} while (0)

#define ASSERT_EQ(a, b, msg)                                                                \
	do {                                                                                \
		uint64_t _va = (uint64_t)(a);                                               \
		uint64_t _vb = (uint64_t)(b);                                               \
		if (_va != _vb) {                                                           \
			dprintf("  FAIL: %s\n", msg);                                       \
			dprintf("    expected: %llu, got: %llu (line %d)\n", (uint64_t)_vb, \
				(uint64_t)_va, __LINE__);                                   \
			tests_failed++;                                                     \
			return;                                                             \
		}                                                                           \
	} while (0)

#define TEST_PASS()                  \
	do {                         \
		dprintf("  PASS\n"); \
		tests_passed++;      \
	} while (0)
