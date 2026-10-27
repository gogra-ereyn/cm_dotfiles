#define CUTE_TEST_IMPL

#include "cute_test.h"
#include "../cute_str.h"

#define CAP 8096

TEST(simple)
{
	char buf[CAP] = { 0 };
	assert_streq(cute_cstr_litquote("hello", buf, sizeof(buf)), "'hello'");
}

TEST(needs_escaping)
{
	char buf[CAP] = { 0 };
	assert_streq(cute_cstr_litquote("hel'lo", buf, sizeof(buf)), "'hel'\\''lo'");
}

TEST(trailing_quote_absorbed)
{
	char buf[CAP] = { 0 };
	assert_streq(cute_cstr_litquote("hello'", buf, sizeof(buf)), "'hello'");
}

TEST(leading_quote_absorbed)
{
	char buf[CAP] = { 0 };
	assert_streq(cute_cstr_litquote("'hello", buf, sizeof(buf)), "'hello'");
}

TEST_MAIN
{
	RUN_TEST(simple);
	RUN_TEST(needs_escaping);
	RUN_TEST(trailing_quote_absorbed);
	RUN_TEST(leading_quote_absorbed);
}
