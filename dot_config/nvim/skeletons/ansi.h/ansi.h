#ifndef ANSI_H
#define ANSI_H
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
	const char *p;
	size_t n;
} span_t;

/* just sketch examples, plagued with snprintf etc */

static int ansi_enabled = 0, ansi_mode = 16;

static void ansi_init(void)
{
	if (!isatty(STDOUT_FILENO))
		return;
	if (getenv("NO_COLOR"))
		return;
	const char *t = getenv("TERM");
	const char *ct = getenv("COLORTERM");
	ansi_enabled = 1;
	if (ct && (!strcmp(ct, "truecolor") || !strcmp(ct, "24bit")))
		ansi_mode = 16777216;
	else if (t && strstr(t, "256color"))
		ansi_mode = 256;
}

static inline span_t sp(const char *s)
{
	return (span_t){ s, s ? strlen(s) : 0 };
}
static inline span_t sp_empty(void)
{
	return (span_t){ NULL, 0 };
}

#define ESC "\x1b["
#define SGR0 "\x1b[0m"

static inline span_t ansi_raw(const char *s)
{
	return ansi_enabled ? sp(s) : sp_empty();
}
static inline span_t ansi_reset(void)
{
	return ansi_raw(SGR0);
}

static inline span_t ansi_sgr1(int a)
{
	static char b[16];
	if (!ansi_enabled) {
		b[0] = 0;
		return sp_empty();
	}
	int n = snprintf(b, sizeof b, ESC "%dm", a);
	return (span_t){ b, (size_t)n };
}

static inline span_t ansi_bold(void)
{
	return ansi_sgr1(1);
}
static inline span_t ansi_dim(void)
{
	return ansi_sgr1(2);
}
static inline span_t ansi_italic(void)
{
	return ansi_sgr1(3);
}
static inline span_t ansi_underline(void)
{
	return ansi_sgr1(4);
}
static inline span_t ansi_inverse(void)
{
	return ansi_sgr1(7);
}

static inline span_t ansi_fg8(int c)
{
	return ansi_sgr1(30 + (c & 7));
}
static inline span_t ansi_bg8(int c)
{
	return ansi_sgr1(40 + (c & 7));
}
static inline span_t ansi_fg_bright(int c)
{
	return ansi_sgr1(90 + (c & 7));
}
static inline span_t ansi_bg_bright(int c)
{
	return ansi_sgr1(100 + (c & 7));
}

static inline span_t ansi_fg256(int i)
{
	static char b[24];
	if (!ansi_enabled) {
		b[0] = 0;
		return sp_empty();
	}
	if (ansi_mode < 256)
		return sp_empty();
	int n = snprintf(b, sizeof b, ESC "38;5;%dm", i & 255);
	return (span_t){ b, (size_t)n };
}
static inline span_t ansi_bg256(int i)
{
	static char b[24];
	if (!ansi_enabled) {
		b[0] = 0;
		return sp_empty();
	}
	if (ansi_mode < 256)
		return sp_empty();
	int n = snprintf(b, sizeof b, ESC "48;5;%dm", i & 255);
	return (span_t){ b, (size_t)n };
}

static inline span_t ansi_fg_rgb(int r, int g, int b)
{
	static char buf[32];
	if (!ansi_enabled) {
		buf[0] = 0;
		return sp_empty();
	}
	if (ansi_mode < 16777216)
		return sp_empty();
	int n = snprintf(buf, sizeof buf, ESC "38;2;%d;%d;%dm", r, g, b);
	return (span_t){ buf, (size_t)n };
}
static inline span_t ansi_bg_rgb(int r, int g, int b)
{
	static char buf[32];
	if (!ansi_enabled) {
		buf[0] = 0;
		return sp_empty();
	}
	if (ansi_mode < 16777216)
		return sp_empty();
	int n = snprintf(buf, sizeof buf, ESC "48;2;%d;%d;%dm", r, g, b);
	return (span_t){ buf, (size_t)n };
}

static inline span_t ansi_cr(void)
{
	return sp("\r");
}
static inline span_t ansi_clear_to_eol(void)
{
	return sp("\x1b[K");
}
static inline span_t ansi_hide_cursor(void)
{
	return ansi_raw(ESC "?25l");
}
static inline span_t ansi_show_cursor(void)
{
	return ansi_raw(ESC "?25h");
}

#endif
