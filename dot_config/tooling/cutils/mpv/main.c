#define _GNU_SOURCE
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>

#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>

#include <string.h>
#include <errno.h>

#define CUTE_LOG_LEVEL CUTE_LOG_ERROR
#include "cute_log.h"

#define DEFAULT_TIMEOUT 100

#define NS_PER_S 1000000000ull
#define NS_PER_MS 1000000ull
#define ALEN(x) (sizeof(x) / sizeof(*x))

/*
 * TODO - initial prog/stats reporting
 *
 *	- [ ] give the progress indicator a buf, don't use libc string formattign
 *	- [ ] revisit potential options for progress indicator if stdout also tty
 *	- [ ] count from first byte (configurably, let users choose)
 *	- [ ] report first byte
 *	- [ ] pretty-printing bytes
 *	- [ ] have the print funcs take ctx, and/or just have the tty flag be passed to progress_t. we want to check both output_fd & the progress_fd to check if they are both heading to the same terminal or not.
 *	- [ ] window-based average throughputs, for ascertaining if eg a rate limit/threshold has been exceeded.
 *	- [ ] pretty printing a la fmb of units
 *	- [X] just bin these sheity doubles for timestamps, do we reckon?
 *	- [ ] epoll_ctl failures when out fd is redirected. ah! splicing stuff maybe
 *	- [ ] block file vs pipe/sock: epoll_ctl fails
 *	- [ ] CONFIRM/check if issues due to nonblocking or not
 *	- [ ] TODO config implementations:
 *	    - impl of 'wait till first byte'
 *	    - window stats
 *	    - formatting
 *	    - tty vs non tty differences
 *	- custom formatters to replace snprintf
 * */

#define MVP_FLAGS_NONE (1u << 0)
#define MVP_FLAGS_TTY (1u << 1)

#define SPLICE_CHUNK (1u << 20)
#define PIPE_SOFT_CAP (1u << 22)

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

char *envstr(char *name)
{
	char *value;
	value = getenv(name);
	return value ? value : "";
}


static uint64_t now_mono_ns(void)
{
	struct timespec ts;
	uint64_t s;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
		cute_die(1, errno, "clock_gettime");
	s = (uint64_t)ts.tv_sec * NS_PER_S + (uint64_t)ts.tv_nsec;
	return s;
}

static int usage(char *name, int rc)
{
	fprintf(stderr, "Usage: %s [options]\n", name);
	fprintf(stderr, "Options:\n");

	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "\t-p, --progress\n");
	fprintf(stderr, "\t\tForce-show progress indicator, irrespective of whether\n");
	fprintf(stderr, "\t\tthe destination fd is tty or not.\n");
	fprintf(stderr, "\t\t[env: MVP_PROGRESS=%s]\n", envstr("MVP_PROGRESS"));

	fprintf(stderr, "\t-I --no-indicator\n");
	fprintf(stderr, "\t\tForce-disable progress indicator, irrespective of whether\n");
	fprintf(stderr, "\t\tthe destination fd is tty or not.\n");
	fprintf(stderr, "\t\t[env: MVP_NO_PROGRESS=%s]\n", envstr("MVP_NO_PROGRESS"));

	fprintf(stderr, "\t-i, --interval-ms\n");
	fprintf(stderr, "\t\tMaximum milliseconds to wait between outputting progress stats\n");
	fprintf(stderr, "\t\t[default: 100]\n");
	fprintf(stderr, "\t\t[env: INVERVAL=%s]\n", envstr("INVERVAL"));

	fprintf(stderr, "\t-w, --window-ms\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "\t--limit-bytes <bytes>\n");
	fprintf(stderr, "\t\tStop after <bytes> have been read+output\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "\t--wait-first\n");
	fprintf(stderr, "\t\tWait until the first byte has been received to start recording.\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "\n");
	fprintf(stderr, "Example:\n");
	fprintf(stderr, "\t%s -a 127.0.0.1 -p 3000\n", name);
	fprintf(stderr, "\n");

	return rc;
}

void invalid_input(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
	usage("main", 1);
}

static int parse_uint64(const char *input, uint64_t *res)
{
	char *endp;
	uint64_t out;
	// not so sure about this1
	errno = 0;
	out = strtoull(input, &endp, 0);
	if (*endp != 0 || errno == ERANGE)
		return 1;
	*res = out;
	return 0;
}

static int parse_int(char *input, int *res)
{
	char *endp;
	long out;
	out = strtol(input, &endp, 0);
	if (*endp != 0)
		return 1;
	*res = (int)out;
	return 0;
}

typedef struct {
	uint64_t bytes_total;
	uint64_t start;
	uint64_t last;

	/* todo - this should probably live with the ctx */
	uint64_t ival;

	uint64_t first_byte;

	/* dest  - todo flags */
	int is_tty;
	int fd;
} progress_t;

typedef struct {
	int ep;
	int ifd;
	int ofd; /* for progress */

	int pipe_wr;
	int pipe_rd;

	size_t pending;

	/* TODO - move all the bools to flags */
	int want_epollout;
	int eof_in;

	int flags;

	progress_t prog;

} ctx_t;

void progress_record(progress_t *p, uint64_t n)
{
	uint64_t now;
	if (!p->bytes_total && n) {
		now = now_mono_ns();
		p->first_byte = now;
	}
	p->bytes_total += n;
}

/* todo - the interval should ac*/
void progress_init(progress_t *p, int fd, uint64_t interval_ms, int indicator)
{
	uint64_t now;
	now = now_mono_ns();
	p->bytes_total = 0;
	p->start = now;
	p->last = now;
	p->is_tty = indicator;
	p->ival = interval_ms * NS_PER_MS;
	p->fd = fd;
}

/* TODO - have format config information in progress_t that we use to change the format  */
static inline ssize_t progress_write_tty(char *dest, int dlen, double elapsed, double rate,
					 progress_t *p)
{
	return snprintf(dest, dlen, "\r[%8.3f s  %12" PRIu64 " B  %10.3f B/s\033[K]", elapsed,
			p->bytes_total, rate);
}

static inline ssize_t progress_write_ntty(char *dest, int dlen, double now, double elapsed,
					  double rate, progress_t *p)
{
	return snprintf(dest, dlen, "ts=%.3f elapsed=%.3f bytes=%" PRIu64 " rate=%.3f\n", now,
			elapsed, p->bytes_total, rate);
}

static void progress_print(progress_t *p, int final)
{
	char buf[128];
	double elapsed, rate, ts_s;
	uint64_t now, elapsed_ns;
	int n;

	now = now_mono_ns();

	if (now >= p->start)
		elapsed_ns = now - p->start;
	else
		elapsed_ns = 0;

	elapsed = (double)elapsed_ns / (double)NS_PER_S;
	if (elapsed > 0.0)
		rate = (double)p->bytes_total / elapsed;
	else
		rate = 0.0;

	ts_s = (double)now / (double)NS_PER_S;
	if (p->is_tty)
		n = progress_write_tty(buf, sizeof buf, elapsed, rate, p);
	else
		n = progress_write_ntty(buf, sizeof buf, ts_s, elapsed, rate, p);

	if (n > 0) {
		if (n > sizeof buf)
			n = sizeof buf;
		write(p->fd, buf, n);
	}

	if (p->is_tty && final)
		write(p->fd, "\n", 1);
}

static void progress_tick(progress_t *p)
{
	uint64_t now;

	now = now_mono_ns();
	if (now - p->last < p->ival)
		return;

	p->last = now;
	progress_print(p, 0);
}

/* todo - appropriately handle if/when not pipes ? */
void ctx_init_epoll(ctx_t *ctx)
{
	struct epoll_event ev;

	ctx->ep = epoll_create1(EPOLL_CLOEXEC);
	if (ctx->ep == -1)
		cute_die(1, errno, "epoll_create1");

	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	ev.data.fd = ctx->ifd;

	if (epoll_ctl(ctx->ep, EPOLL_CTL_ADD, ctx->ifd, &ev) == -1)
		cute_error("epoll ctl add for ifd failed");

	ev.events = 0;
	ev.data.fd = ctx->ofd;
	if (epoll_ctl(ctx->ep, EPOLL_CTL_ADD, ctx->ofd, &ev) == -1)
		cute_error("epoll ctl add for ofd failed");
}

void ctx_init_pipe(ctx_t *ctx)
{
	int p[2];
	if (pipe2(p, O_CLOEXEC | O_NONBLOCK) == -1)
		cute_die(1, errno, "pipe2");

	ctx->pipe_rd = p[0];
	ctx->pipe_wr = p[1];

	/* not weird if no sudo priv for this to fail */
	fcntl(ctx->pipe_rd, F_SETPIPE_SZ, PIPE_SOFT_CAP);
}

int set_nb(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return -1;
	return 0;
}

void ctx_update_epollout(ctx_t *ctx)
{
	struct epoll_event ev;
	ev.data.fd = ctx->ofd;
	ev.events = EPOLLERR | EPOLLHUP;
	if (ctx->pending > 0)
		ev.events |= EPOLLOUT;
	if (epoll_ctl(ctx->ep, EPOLL_CTL_MOD, ctx->ofd, &ev) == -1)
		cute_info("epoll ctl on ofd fail");
	ctx->want_epollout = (ctx->pending > 0);
}

int handle_in(ctx_t *ctx)
{
	ssize_t n;
	size_t room;

	for (;;) {
		if (ctx->pending >= PIPE_SOFT_CAP)
			break;

		room = PIPE_SOFT_CAP - ctx->pending;
		if (room > SPLICE_CHUNK)
			room = SPLICE_CHUNK;

		n = splice(ctx->ifd, NULL, ctx->pipe_wr, NULL, room,
			   SPLICE_F_MOVE | SPLICE_F_MORE | SPLICE_F_NONBLOCK);

		if (n > 0) {
			ctx->pending += n;
			progress_record(&ctx->prog, n);
			continue;
		}

		if (n == 0) {
			ctx->eof_in = 1;
			break;
		}

		if (errno == EAGAIN)
			break;

		if (errno == EINTR)
			continue;

		cute_die(1, errno, "splice(ifd->pipe)");
	}

	if (ctx->pending > 0 && !ctx->want_epollout)
		ctx_update_epollout(ctx);

	return 0;
}

int handle_out(ctx_t *ctx)
{
	ssize_t n;
	while (ctx->pending > 0) {
		n = splice(ctx->pipe_rd, NULL, ctx->ofd, NULL, ctx->pending,
			   SPLICE_F_MOVE | SPLICE_F_MORE | SPLICE_F_NONBLOCK);
		if (n > 0) {
			ctx->pending -= (size_t)n;
			continue;
		}

		if (errno == EAGAIN || errno == EWOULDBLOCK)
			break;

		if (errno == EINTR)
			continue;

		cute_die(1, errno, "splice(pipe->ofd)");
	}

	/* TODO - there's for sure something awry with this when
	 *	 the output fd is not a pipe
	 * */
	if (ctx->pending == 0 && ctx->want_epollout)
		ctx_update_epollout(ctx);

	return 0;
}

int main_loop(ctx_t *ctx, int timeout_ms)
{
	struct epoll_event evs[1024];
	int i, n, finito;
	finito = 0;

	for (; !finito;) {
		n = epoll_wait(ctx->ep, evs, ALEN(evs), timeout_ms);

		cute_debug("epoll wait=%d", n);

		if (n == -1) {
			if (errno == EINTR)
				continue;
			cute_die(1, errno, "epoll_wait");
		}

		if (n == 0)
			continue;

		for (i = 0; i < n; ++i) {
			if (evs[i].data.fd == ctx->ifd) {
				if (evs[i].events & (EPOLLERR | EPOLLHUP))
					ctx->eof_in = 1;

				if (evs[i].events & EPOLLIN)
					handle_in(ctx);

			} else if (evs[i].data.fd == ctx->ofd) {
				if (evs[i].events & (EPOLLERR | EPOLLHUP))
					finito = 1;

				if (evs[i].events & EPOLLOUT)
					handle_out(ctx);
			}
		}

		handle_out(ctx);
		progress_tick(&ctx->prog);
		finito = ctx->eof_in && ctx->pending == 0;
	}

	progress_print(&ctx->prog, 1);

	return 0;
}

static int same_tty(int fd1, int fd2)
{
	struct stat st1, st2;
	if (!isatty(fd1) || !isatty(fd2))
		return 0;
	if (fstat(fd1, &st1) == -1 || fstat(fd2, &st2) == -1)
		return 0;
	return (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino);
}

int main(int argc, char **argv)

{
	uint64_t ival_ms = 0;
	int ifd = 0, ofd = 1;
	int force_progress = 0;
	int force_set = 0;
	ctx_t ctx = { 0 };
	int timeout_ms = DEFAULT_TIMEOUT;
	static struct option long_options[] = { { "help", no_argument, 0, 'h' },
						{ "timeout_ms", required_argument, 0, 't' },
						{ "interval_ms", required_argument, 0, 'i' },
						{ "input", required_argument, 0, 'f' },
						{ "limit_bytes", required_argument, 0, 'b' },
						{ "no_indicator", no_argument, 0, 'I' },
						{ "progress", no_argument, 0, 'p' },
						{ 0, 0, 0, 0 } };

	/* todo: detect '--' for command */
	int c, option_index = 0;
	while ((c = getopt_long(argc, argv, "ha:t:f:i:b:Ip", long_options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(*argv, 0);
			return 0;
		case 'f':
			ifd = open(optarg, O_RDONLY);
			if (ifd < 0)
				invalid_input("Error opening provided input file: '%s'", optarg);
			break;
		case 'p':
			force_set = 1;
			force_progress = 1;
			break;
		case 'I':
			force_set = 1;
			force_progress = 0;
			break;
		case 'b':
			if (parse_uint64(optarg, &ival_ms))
				invalid_input("invalid ms value for interval-ms: '%s'", optarg);
			break;
		case 'o':
			ofd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if (ofd == -1)
				invalid_input("Error openeing file at provided output path: '%s'",
					      optarg);
			break;
		case 't':
			if (parse_int(optarg, &timeout_ms))
				invalid_input("Invalid int format for timeout: '%s'", optarg);
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	struct stat st;
	if (fstat(ifd, &st) == -1)
		invalid_input("Failed to fstat fd=%d", ifd);

	if (S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode))
		cute_info("Input fd=%d is a pipe! good.", ifd);
	else
		cute_die(1, 0, "input fd=%d is not pipe. this is pipe viewer tool. wat r u doing?",
			ifd);

	ctx.ifd = ifd;
	ctx.ofd = ofd;

	force_progress = force_set ? force_progress :
				     (isatty(2) && !isatty(ifd)) || !same_tty(2, ifd);

	/* todo - define defaults, cli/env args override */
	progress_init(&ctx.prog, 2, ival_ms, force_progress);
	ctx_init_epoll(&ctx);
	ctx_init_pipe(&ctx);
	return main_loop(&ctx, timeout_ms);
}
