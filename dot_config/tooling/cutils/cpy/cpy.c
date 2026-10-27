#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "cute_log.h"

#define CCP_PFX "\x1B]52;c;"
#define CCP_PFX_LEN (sizeof(CCP_PFX) - 1)
#define CCP_SFX '\x07'

/* not paging in what i dont need  */
#define CCP_IN_MAX (1u << 27)

/* input region + output region (b64 expands by ~4/3 + prefix/suffix) */
#define CCP_OUT_MAX (CCP_PFX_LEN + (((CCP_IN_MAX + 2u) / 3u) * 4u) + 1u)
#define CCP_BUFMAX (CCP_IN_MAX + CCP_OUT_MAX)

const uint8_t encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int encode(const uint8_t *in, size_t len, uint8_t *out_buf, size_t *outlen)
{
	if (!in || !len) {
		cute_error("input is empty. wont bother.");
		return 1;
	}

	const uint8_t *l = in;
	const uint8_t *r = in + len;
	uint8_t *el = out_buf;
	size_t bv;

	while (l < r) {
		bv = *l++;
		bv = l < r ? bv << 8 | *l : bv << 8;
		bv = l + 1 < r ? bv << 8 | *(l + 1) : bv << 8;

		*el++ = encoding_table[(bv >> 18) & 0x3F];
		*el++ = encoding_table[(bv >> 12) & 0x3F];

		if (l++ < r)
			*el++ = encoding_table[(bv >> 6) & 0x3F];
		else
			*el++ = '=';

		if (l++ < r)
			*el++ = encoding_table[bv & 0x3F];
		else
			*el++ = '=';
	}

	*outlen = (el - out_buf);
	return 0;
}

size_t cute_b64_encoded_cap(size_t in_len)
{
	size_t n;
	n = (in_len + 2) / 3;
	return n * 4;
}

size_t cute_osc52_encoded_cap(size_t in_len)
{
	return CCP_PFX_LEN + cute_b64_encoded_cap(in_len);
}

size_t cute_trim_eol(uint8_t *buf, size_t len)
{
	size_t n;
	uint8_t c;
	n = len;
	while (n > 0) {
		c = buf[n - 1];
		if (c == '\n' || c == '\r')
			n--;
		else
			break;
	}
	return n;
}

static int write_all(int fd, const uint8_t *buf, size_t len)
{
	ssize_t bwrit;
	const uint8_t *p;
	size_t off;

	p = buf;
	off = 0;
	while (off < len) {
		bwrit = write(fd, p + off, len - off);
		if (bwrit < 0) {
			if (errno == EINTR)
				continue;
			return 1;
		}
		if (bwrit == 0)
			return 1;
		off += bwrit;
	}
	return 0;
}

void cute_osc52_build(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len)
{
	size_t b64_len;

	if (!out_len)
		cute_die(1, 0, "yo boss you gave me an uninit ptr for *out_len");

	*out_len = 0;

	if (!in || !out)
		cute_die(1, 0, "cute_osc52_build given a null buf ptr. nice1.");

	memcpy(out, CCP_PFX, CCP_PFX_LEN);

	b64_len = 0;
	encode(in, in_len, out + CCP_PFX_LEN, &b64_len);

	out[CCP_PFX_LEN + b64_len] = CCP_SFX;
	*out_len = CCP_PFX_LEN + b64_len + 1u;
}

static size_t read_fd(int fd, uint8_t *buf, size_t cap)
{
	ssize_t bread;
	size_t off;

	off = 0;
	while (off < cap) {
		bread = read(fd, buf + off, cap - off);
		if (bread == -1) {
			if (errno == EINTR)
				continue;
			cute_die(1, errno, "failed to read input");
		}
		if (bread == 0)
			break;
		off += bread;
	}

	if (off == cap) {
		bread = read(fd, buf, 1);
		if (bread)
			cute_die(1, 0, "input exceeds capacity");
	}

	return off;
}

static int exec_pipe(char **argv)
{
	int pipefd[2];
	pid_t pid;

	if (pipe(pipefd))
		cute_die(1, errno, "pipe failed");

	pid = fork();
	if (pid < 0)
		cute_die(1, errno, "fork failed");

	if (pid == 0) {
		close(pipefd[0]);
		dup2(pipefd[1], 1);
		close(pipefd[1]);
		execvp(argv[0], argv);
		_exit(127);
	}

	close(pipefd[1]);
	return pipefd[0];
}

static int usage(char *name, int rc)
{
	dprintf(2, "Usage: %s [options] [FILE]\n", name);
	dprintf(2, "       %s -e [options] -- CMD [ARGS...]\n", name);
	dprintf(2, "\nCopy input to clipboard via OSC 52.\n\n");
	dprintf(2, "Options:\n");

	dprintf(2, "\t-h, --help\n");
	dprintf(2, "\t\tShow this help message\n");
	dprintf(2, "\n");

	dprintf(2, "\t-e, --exec\n");
	dprintf(2, "\t\tExecute the given command and copy its stdout.\n");
	dprintf(2, "\n");

	dprintf(2, "\t-k, --keep-newlines\n");
	dprintf(2,
		"\t\tDo not trim trailing newline characters from input (the default behavior).\n");
	dprintf(2, "\n");
	dprintf(2, "\t-n, --newline\n");
	dprintf(2, "\t\tAppend a newline after the outputted osc52 encoded string.\n");
	dprintf(2, "\t\tNot useful for standard usage, but can be helpful when not being output\n");
	dprintf(2, "\t\tto a terminal.\n");
	dprintf(2, "\n");

	return rc;
}

int main(int argc, char **argv)
{
	uint8_t *buf, *in_buf, *out_buf;
	size_t trimmed, out_len, off;
	int rc, input_fd;
	pid_t child_pid = -1;

	int keep_nl = 0;
	int append_nl = 0;
	int exec_mode = 0;
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "version", no_argument, 0, 'v' },
		{ "exec", no_argument, 0, 'e' },
		{ "keep-newlines", no_argument, 0, 'k' },
		{ "newline", no_argument, 0, 'n' },
		{ 0, 0, 0, 0 },
	};

	int c, option_index;

	while ((c = getopt_long(argc, argv, "hvekn", long_options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			return usage(*argv, 0);
		case 'v':
			dprintf(1, "cpy %s\n", CPY_VERSION);
			return 0;
		case 'e':
			exec_mode = 1;
			break;
		case 'k':
			keep_nl = 1;
			break;
		case 'n':
			append_nl = 1;
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	buf = malloc(CCP_BUFMAX);
	if (!buf)
		cute_die(1, errno, "failed to malloc our buffywuffy");

	in_buf = buf;
	out_buf = buf + CCP_IN_MAX;

	if (!exec_mode && optind > 1 && strcmp(argv[optind - 1], "--") == 0)
		exec_mode = 1;

	if (exec_mode) {
		if (optind >= argc)
			cute_die(1, 0, "exec mode requires a command");
		input_fd = exec_pipe(argv + optind);
		child_pid = 0;
	} else if (optind < argc) {
		if (optind + 1 < argc)
			cute_die(1, 0, "expected a single file argument");
		input_fd = open(argv[optind], O_RDONLY);
		if (input_fd < 0)
			cute_die(1, errno, "failed to open %s", argv[optind]);
	} else {
		input_fd = 0;
	}

	off = read_fd(input_fd, in_buf, CCP_IN_MAX);

	if (input_fd > 0)
		close(input_fd);

	if (child_pid == 0) {
		int status;
		if (wait(&status) < 0)
			cute_die(1, errno, "wait failed");
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			cute_die(1, 0, "command exited with status %d", WEXITSTATUS(status));
		if (WIFSIGNALED(status))
			cute_die(1, 0, "command killed by signal %d", WTERMSIG(status));
	}

	trimmed = keep_nl ? off : cute_trim_eol(in_buf, off);
	if (!trimmed)
		cute_die(1, 0, "input is empty");

	out_len = 0;
	cute_osc52_build(in_buf, trimmed, out_buf, &out_len);

	rc = write_all(1, out_buf, out_len);
	if (rc)
		cute_die(1, errno, "failed to write encoded bytes to stdout");

	if (append_nl)
		write(1, "\n", 1);

	return 0;
}
