#ifndef SNOWFLAKE_CORE_H
#define SNOWFLAKE_CORE_H

#include <stdint.h>

#define SF_EPOCH_MILLIS 1700000000000ULL

enum { SF_NODE_BITS = 10, SF_SEQ_BITS = 12 };
enum {
	SF_NODE_MAX = (1U << SF_NODE_BITS) - 1,
	SF_SEQ_MAX = (1U << SF_SEQ_BITS) - 1
};

typedef struct {
	uint64_t ts;
	uint16_t node_id;
	uint16_t seq;
} sf_parts_t;

typedef struct {
	uint16_t node_id;
	uint16_t seq;
	uint64_t last_ms;
} sf_gen_t;

static inline uint64_t snowflake_compose(uint64_t ts, uint16_t node_id,
					 uint16_t seq)
{
	return ((ts - SF_EPOCH_MILLIS) << (SF_NODE_BITS + SF_SEQ_BITS)) |
	       ((uint64_t)(node_id & SF_NODE_MAX) << SF_SEQ_BITS) |
	       (uint64_t)(seq & SF_SEQ_MAX);
}

static inline uint64_t snowflake_gen_next(uint64_t *last_ms, uint16_t node_id,
					  uint16_t *seq, uint64_t now_ms)
{
	if (now_ms < *last_ms)
		now_ms = *last_ms;

	if (now_ms == *last_ms) {
		if (++(*seq) > SF_SEQ_MAX) {
			now_ms++;
			*seq = 0;
		}
	} else {
		*seq = 0;
	}

	*last_ms = now_ms;

	return snowflake_compose(now_ms, node_id, *seq);
}

void sf_gen_init(sf_gen_t *c, uint16_t node_id)
{
	c->node_id = node_id & SF_NODE_MAX;
	c->seq = 0;
	c->last_ms = 0;
}

uint64_t sf_gen_next(sf_gen_t *c, uint64_t now_ms)
{
	return snowflake_gen_next(&c->last_ms, c->node_id, &c->seq, now_ms);
}

void sf_extract_parts(uint64_t id, sf_parts_t *parts)
{
	parts->ts = (id >> (SF_NODE_BITS + SF_SEQ_BITS)) + SF_EPOCH_MILLIS;
	parts->node_id = (id >> SF_SEQ_BITS) & SF_NODE_MAX;
	parts->seq = id & SF_SEQ_MAX;
}

void sf_extract_raw_parts(uint64_t id, uint64_t *ts, uint16_t *node_id,
			  uint16_t *seq)
{
	*ts = (id >> (SF_NODE_BITS + SF_SEQ_BITS)) + SF_EPOCH_MILLIS;
	*node_id = (id >> SF_SEQ_BITS) & SF_NODE_MAX;
	*seq = id & SF_SEQ_MAX;
}

#endif

