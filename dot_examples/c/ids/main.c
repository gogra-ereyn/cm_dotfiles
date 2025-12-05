#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include "sf.h"

static uint64_t now_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void print_id(uint64_t id)
{
	sf_parts_t parts;

	sf_extract_parts(id, &parts);
	printf("id: %lu (0x%016lx)\n\n", id, id);
	printf("timestamp (ms): %lu\n", parts.ts);
	printf("node_id:        %u\n", parts.node_id);
	printf("seq:            %u\n", parts.seq);
}

static void usage(const char *prog, int rc)
{
	fprintf(stderr, "usage: %s [-t timestamp_ms] [-n node_id] [-s seq]\n",
		prog);
	fprintf(stderr, "       %s -d <id>\n", prog);
	fprintf(stderr, "       %s -m <id> [-t ts] [-n node] [-s seq]\n", prog);
	fprintf(stderr, "\ngenerate mode (default):\n");
	fprintf(stderr, "  -t, --timestamp  timestamp in ms (default: now)\n");
	fprintf(stderr, "  -n, --node       node id 0-%u (default: 42)\n",
		SF_NODE_MAX);
	fprintf(stderr, "  -s, --seq        sequence 0-%u (default: 0)\n",
		SF_SEQ_MAX);
	fprintf(stderr, "\ndecode mode:\n");
	fprintf(stderr, "  -d, --decode     decode an existing snowflake id\n");
	fprintf(stderr, "\nmodify mode:\n");
	fprintf(stderr,
		"  -m, --modify     modify an existing id, use -t/-n/-s to set fields\n");
	exit(rc);
}

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "timestamp", required_argument, 0, 't' },
		{ "node", required_argument, 0, 'n' },
		{ "seq", required_argument, 0, 's' },
		{ "decode", required_argument, 0, 'd' },
		{ "modify", required_argument, 0, 'm' },
		{ 0, 0, 0, 0 }
	};
	int c, option_index = 0;
	uint64_t ts = 0;
	uint16_t node_id = 42;
	uint16_t seq = 0;
	int ts_set = 0, node_set = 0, seq_set = 0;
	uint64_t id;
	uint64_t target_id = 0;
	int decode_mode = 0, modify_mode = 0;
	sf_parts_t parts;

	while ((c = getopt_long(argc, argv, "ht:n:s:d:m:", long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0], 0);
		case 't':
			ts = strtoull(optarg, NULL, 10);
			ts_set = 1;
			break;
		case 'n':
			node_id = (uint16_t)strtoul(optarg, NULL, 10);
			node_set = 1;
			break;
		case 's':
			seq = (uint16_t)strtoul(optarg, NULL, 10);
			seq_set = 1;
			break;
		case 'd':
			target_id = strtoull(optarg, NULL, 10);
			decode_mode = 1;
			break;
		case 'm':
			target_id = strtoull(optarg, NULL, 10);
			modify_mode = 1;
			break;
		case '?':
			return 1;
		default:
			break;
		}
	}

	if (decode_mode) {
		print_id(target_id);
		return 0;
	}

	if (modify_mode) {
		sf_extract_parts(target_id, &parts);
		if (ts_set)
			parts.ts = ts;
		if (node_set)
			parts.node_id = node_id;
		if (seq_set)
			parts.seq = seq;
		id = snowflake_compose(parts.ts, parts.node_id, parts.seq);
		print_id(id);
		return 0;
	}

	if (!ts_set)
		ts = now_ms();

	id = snowflake_compose(ts, node_id, seq);
	print_id(id);

	return 0;
}

