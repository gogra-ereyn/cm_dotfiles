#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "../massert.h"
#include "../http.h"
#include "../core.h"

static int use_color(void)
{
	const char *nc;
	nc = getenv("NO_COLOR");
	if (nc && *nc)
		return 0;
	if (!isatty(2))
		return 0;
	return 1;
}

static void writef(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void run_test(const char *name, void (*fn)(void), int *passcnt,
		     int *failcnt)
{
	int color;
	const char *ok, *bad, *dim, *rst;
	struct timespec t0, t1;
	long ms;
	pid_t pid;
	int status;
	color = use_color();
	ok = color ? "\x1b[32m✔\x1b[0m" : "OK";
	bad = color ? "\x1b[31m✘\x1b[0m" : "FAIL";
	dim = color ? "\x1b[90m" : "";
	rst = color ? "\x1b[0m" : "";
	clock_gettime(CLOCK_MONOTONIC, &t0);
	pid = fork();
	if (pid == 0) {
		fn();
		_exit(0);
	}
	waitpid(pid, &status, 0);
	clock_gettime(CLOCK_MONOTONIC, &t1);
	ms = (t1.tv_sec - t0.tv_sec) * 1000 +
	     (t1.tv_nsec - t0.tv_nsec) / 1000000;
	if ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
	    WIFSIGNALED(status) == 0) {
		(*passcnt)++;
		writef("%s %s %s%ldms%s\n", ok, name, dim, ms, rst);
	} else {
		(*failcnt)++;
		writef("%s %s %s%ldms%s\n", bad, name, dim, ms, rst);
	}
}
static void test_url_decode_inplace_basic(void)
{
	char s1[64];
	char s2[64];
	char s3[64];
	strcpy(s1, "hello%20world+test");
	url_decode_inplace(s1);
	assert_ex(strcmp(s1, "hello world test") == 0);
	strcpy(s2, "%41%42%43");
	url_decode_inplace(s2);
	assert_ex(strcmp(s2, "ABC") == 0);
	strcpy(s3, "plain");
	url_decode_inplace(s3);
	assert_ex(strcmp(s3, "plain") == 0);
}

static void test_url_decode_inplace_malformed(void)
{
	char a[16];
	char b[16];
	char c[16];
	char d[16];
	strcpy(a, "%");
	url_decode_inplace(a);
	assert_ex(strcmp(a, "%") == 0);
	strcpy(b, "%G1");
	url_decode_inplace(b);
	assert_ex(strcmp(b, "%G1") == 0);
	strcpy(c, "%4");
	url_decode_inplace(c);
	assert_ex(strcmp(c, "%4") == 0);
	strcpy(d, "x%2");
	url_decode_inplace(d);
	assert_ex(strcmp(d, "x%2") == 0);
}

static void test_url_decode_inplace_case_plus(void)
{
	char l[16];
	char u[16];
	char p[32];
	strcpy(l, "%2f");
	url_decode_inplace(l);
	assert_ex(strcmp(l, "/") == 0);
	strcpy(u, "%2F");
	url_decode_inplace(u);
	assert_ex(strcmp(u, "/") == 0);
	strcpy(p, "++a+b++");
	url_decode_inplace(p);
	assert_ex(strcmp(p, "  a b  ") == 0);
}

static void test_qp_get_basic(void)
{
	char q[256];
	char out[128];
	strcpy(q, "user=alice+smith&uuid=123e4567%2De89b&x=1");
	assert_ex(qp_get(q, "user", out, sizeof(out)));
	assert_ex(strcmp(out, "alice smith") == 0);
	assert_ex(qp_get(q, "uuid", out, sizeof(out)));
	assert_ex(strcmp(out, "123e4567-e89b") == 0);
	assert_ex(!qp_get(q, "missing", out, sizeof(out)));
}

static void test_qp_get_repeated_prefix_empty(void)
{
	char q1[256];
	char out[128];
	strcpy(q1, "u=1&user=alice&u=2");
	assert_ex(qp_get(q1, "u", out, sizeof(out)));
	assert_ex(strcmp(out, "1") == 0);
	assert_ex(qp_get(q1, "user", out, sizeof(out)));
	assert_ex(strcmp(out, "alice") == 0);

	strcpy(q1, "a=&b=2&&");
	assert_ex(qp_get(q1, "a", out, sizeof(out)));
	assert_ex(out[0] == 0);
}

static void test_qp_get_exact_fit_and_overflow(void)
{
	char q[64];
	char out1[5];
	char out2[4];
	strcpy(q, "k=abcd");
	assert_ex(qp_get(q, "k", out1, sizeof(out1)));
	assert_ex(strcmp(out1, "abcd") == 0);
	assert_ex(!qp_get(q, "k", out2, sizeof(out2)));
}

static void test_qp_get_key_prefix_safety(void)
{
	char q[128];
	char out[32];
	strcpy(q, "user=alice&u=bob");
	assert_ex(qp_get(q, "u", out, sizeof(out)));
	assert_ex(strcmp(out, "bob") == 0);
	assert_ex(qp_get(q, "user", out, sizeof(out)));
	assert_ex(strcmp(out, "alice") == 0);
}

static void test_atoi_helpers_boundaries(void)
{
	assert_ex(http_to_i64("9223372036854775807") == 9223372036854775807LL);
	assert_ex(http_to_i64("-9223372036854775808") ==
		  (-9223372036854775807LL - 1));
	assert_ex(http_to_i64("9223372036854775808") == -1);
	assert_ex(http_to_i32("2147483647") == 2147483647);
	assert_ex(http_to_i32("-2147483648") == (-2147483647 - 1));
	assert_ex(http_to_i32("2147483648") == -1);
}

#define RUN(name) run_test(#name, name, &passcnt, &failcnt)

int main(void)
{
	int passcnt;
	int failcnt;
	int color;
	const char *hdr, *rst;
	passcnt = 0;
	failcnt = 0;
	color = use_color();
	hdr = color ? "\x1b[36m" : "";
	rst = color ? "\x1b[0m" : "";
	writef("%shttp unit tests%s\n", hdr, rst);
	RUN(test_url_decode_inplace_basic);
	RUN(test_url_decode_inplace_malformed);
	RUN(test_url_decode_inplace_case_plus);
	RUN(test_qp_get_basic);
	RUN(test_qp_get_repeated_prefix_empty);
	RUN(test_qp_get_exact_fit_and_overflow);
	RUN(test_qp_get_key_prefix_safety);
	RUN(test_atoi_helpers_boundaries);
	writef("summary %d/%d passed\n", passcnt, passcnt + failcnt);
	return failcnt ? 1 : 0;
}
