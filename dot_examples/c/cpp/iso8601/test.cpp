#include <cstdint>
#include <cstdio>
#include <chrono>

#include "parse_iso8601_ms.hpp"

static int g_failures = 0;

static void fail_line(const char *expr, const char *file, int line)
{
	++g_failures;
	std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, expr);
}

#define CHECK(expr)                                           \
	do {                                                  \
		if (!(expr))                                  \
			fail_line(#expr, __FILE__, __LINE__); \
	} while (0)

static int64_t sec_ms(int64_t s)
{
	return s * 1000LL;
}

static void check_ok_ms(const char *s, int64_t expected_ms)
{
	int64_t got;
	bool ok;
	got = 0;
	ok = iso8601_ms::parse_ms_since_epoch(s, got);
	CHECK(ok);
	if (ok)
		CHECK(got == expected_ms);
}

static void check_ok_eq(const char *a, const char *b)
{
	int64_t ma, mb;
	bool oka, okb;
	ma = 0;
	mb = 0;
	oka = iso8601_ms::parse_ms_since_epoch(a, ma);
	okb = iso8601_ms::parse_ms_since_epoch(b, mb);
	CHECK(oka);
	CHECK(okb);
	if (oka && okb)
		CHECK(ma == mb);
}

static void check_bad(const char *s)
{
	int64_t got;
	bool ok;
	got = 123;
	ok = iso8601_ms::parse_ms_since_epoch(s, got);
	CHECK(!ok);
}

int main()
{
	check_ok_ms("1970-01-01T00:00:00Z", 0);
	check_ok_ms("1970-01-01T00:00:00+0000", 0);
	check_ok_ms("1970-01-01T01:00:00+0100", 0);
	check_ok_ms("1970-01-01T00:00:00-0100", sec_ms(3600));
	check_ok_ms("1969-12-31T23:59:59Z", sec_ms(-1));

	check_ok_ms("1970-01-01T00:00:00.1Z", 100);
	check_ok_ms("1970-01-01T00:00:00.01Z", 10);
	check_ok_ms("1970-01-01T00:00:00.001Z", 1);
	check_ok_ms("1970-01-01T00:00:00.000Z", 0);
	check_ok_ms("1970-01-01T00:00:00.123Z", 123);
	check_ok_ms("1970-01-01T00:00:00.1234Z", 123);
	check_ok_ms("1970-01-01T00:00:00.999999999Z", 999);

	check_ok_eq("1970-01-01T00:00:00Z", "1970-01-01 00:00:00Z");
	check_ok_eq("1970-01-01T00:00:00+00:00", "1970-01-01T00:00:00+0000");
	check_ok_eq("1970-01-01T00:00:00-05:30", "1970-01-01T05:30:00Z");
	check_ok_eq("1970-01-01T00:00:00+05:30", "1969-12-31T18:30:00Z");

	check_ok_eq("2000-02-29T12:34:56Z", "2000-02-29T12:34:56+0000");
	check_ok_eq("1999-12-31T23:59:59Z", "2000-01-01T00:59:59+0100");

	check_bad("");
	check_bad("1970-01-01T00:00:00");
	check_bad("1970-01-01T00:00:00.Z");
	check_bad("1970-01-01T00:00:00.AZ");
	check_bad("1970-01-01T00:00:00.123");
	check_bad("1970-01-01T00:00:00+");
	check_bad("1970-01-01T00:00:00+000");
	check_bad("1970-01-01T00:00:00+00");
	check_bad("1970-01-01T00:00:00+24:00");
	check_bad("1970-01-01T00:00:00+23:60");

	check_bad("1970/01/01T00:00:00Z");
	check_bad("1970-01-01X00:00:00Z");

	check_bad("1970-00-01T00:00:00Z");
	check_bad("1970-13-01T00:00:00Z");
	check_bad("1970-01-00T00:00:00Z");
	check_bad("1970-01-32T00:00:00Z");
	check_bad("1970-02-29T00:00:00Z");
	check_bad("2001-02-29T00:00:00Z");

	check_bad("1970-01-01T24:00:00Z");
	check_bad("1970-01-01T00:60:00Z");
	check_bad("1970-01-01T00:00:61Z");

	check_bad("1970-01-01T00:00:00Z ");
	check_bad("1970-01-01T00:00:00Zjunk");

	if (g_failures == 0) {
		std::printf("OK\n");
		return 0;
	}

	std::fprintf(stderr, "%d failure(s)\n", g_failures);
	return 1;
}
