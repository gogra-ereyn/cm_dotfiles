#define _GNU_SOURCE
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

static void writef(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

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

#define RUN(name) run_test(#name, name, &passcnt, &failcnt)

int example_func_to_test(const char s)
{
	return 0;
}

// need dis skele sir
#include "massert.h"
int _example_main(int argc, char **argv)
{
	assert_eq(example_func_to_test(""), -1);
	assert_eq(example_func_to_test(" "), -1);
	assert_eq(example_func_to_test("+1"), 1);
	assert_eq(example_func_to_test("01"), 1);
	assert_eq(example_func_to_test("1x"), -1);
}
