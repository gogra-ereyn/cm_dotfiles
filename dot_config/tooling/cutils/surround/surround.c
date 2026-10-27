#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <stdint.h>
#include "cute_log.h"

/* TODO or TOCONSIDER
 *
 * - [ ] Handling bash-aware string surrounds: cases for when a terminating surround character exists
 *	 in the input string/word already.
 *	 e.g. if we have the word - it's - and we do a surround with '
 *	    almost all of the time in a CLI context we would not want the resulting
 *	    unpaired "'".
 *	    TO EXPLORE: how best to handle this, if we can do by escaping or by choosing
 *			alternative quotes e.g. "
 *
 * - [ ] Here or other binary for e.g. stringifying smarly every bash-word in the input. The idea would be to surround each space-spearated word in the input with either ' or ", depending on whether there is a blatant/inferred variable (eg /path/to/$filename) expansion going on.
 *
 *
 */

#define LINE_MAX_BUF 8192

/* paired delimiters: open -> close */
static const char pair_open[] = "([{<";
static const char pair_close[] = ")]}>";

static char match_close(char c)
{
	const char *p = strchr(pair_open, c);
	if (p)
		return pair_close[p - pair_open];
	p = strchr(pair_close, c);
	if (p)
		return pair_open[p - pair_close];
	return c;
}

/* word: delimited by non-alnum/underscore (vim iw) */
static int is_word_char(uint8_t c)
{
	return isalnum(c) || c == '_';
}

/* WORD: delimited by whitespace only (vim iW) */
static int is_WORD_char(uint8_t c)
{
	return !isspace(c);
}

static int usage(const char *name, int rc)
{
	dprintf(2, "Usage: %s -p CURSOR [-W] CHAR\n", name);
	dprintf(2, "  reads line from stdin, surrounds word under cursor with CHAR\n");
	dprintf(2, "  outputs: new_line\\nnew_cursor\\n\n");
	dprintf(2, "Options:\n");
	dprintf(2, "\t-p POS   cursor byte offset (required)\n");
	dprintf(2, "\t-W       use WORD boundaries (whitespace-delimited)\n");
	dprintf(2, "\t-h       show this help\n");
	return rc;
}

int main(int argc, char **argv)
{
	char line[LINE_MAX_BUF];
	char out[LINE_MAX_BUF + 4]; /* room for 2 surround chars */
	ssize_t bread;
	size_t len;
	int cursor = -1;
	int bigword = 0;
	int c;

	int (*boundary)(uint8_t);

	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 },
	};
	int option_index;

	while ((c = getopt_long(argc, argv, "hp:W", long_options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			return usage(*argv, 0);
		case 'p':
			cursor = atoi(optarg);
			break;
		case 'W':
			bigword = 1;
			break;
		case '?':
			return 1;
		}
	}

	if (cursor < 0)
		cute_die(1, 0, "cursor position required (-p)");

	if (optind >= argc)
		cute_die(1, 0, "surround character required");

	char open_ch = argv[optind][0];
	char close_ch = match_close(open_ch);

	/* read line from stdin */
	len = 0;
	while (len < sizeof(line) - 1) {
		bread = read(0, line + len, sizeof(line) - 1 - len);
		if (bread < 0) {
			if (errno == EINTR)
				continue;
			cute_die(1, errno, "read");
		}
		if (bread == 0)
			break;
		len += bread;
	}

	/* strip trailing newline */
	while (len > 0 && line[len - 1] == '\n')
		len--;
	line[len] = 0;

	if ((size_t)cursor > len)
		cursor = len;

	boundary = bigword ? is_WORD_char : is_word_char;

	/* find word boundaries around cursor */
	int ws = cursor;
	int we = cursor;

	/* expand left */
	while (ws > 0 && boundary((uint8_t)line[ws - 1]))
		ws--;

	/* expand right */
	while ((size_t)we < len && boundary((uint8_t)line[we]))
		we++;

	/* if cursor is not on a word char, just insert at cursor */
	if (ws == we) {
		ws = cursor;
		we = cursor;
	}

	/* build output: line[0..ws] + open + line[ws..we] + close + line[we..] */
	size_t olen = 0;
	memcpy(out, line, ws);
	olen = ws;
	out[olen++] = open_ch;
	memcpy(out + olen, line + ws, we - ws);
	olen += we - ws;
	out[olen++] = close_ch;
	memcpy(out + olen, line + we, len - we);
	olen += len - we;
	out[olen] = 0;

	/* new cursor: just after the opening char */
	int new_cursor = ws + 1;

	dprintf(1, "%s\n%d\n", out, new_cursor);

	return 0;
}
