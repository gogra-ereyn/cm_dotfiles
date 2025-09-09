#ifndef SNOWFLAKE_EX_H
#define SNOWFLAKE_EX_H

#define _POSIX_C_SOURCE 200809L

#include "snowflake_example_mtx.h"
#include <time.h>
#include <errno.h>
#include <stdatomic.h>

#include "../snowflake.h/snowflake.h"
#include <pthread.h>

typedef struct {
	uint16_t node_id;
	uint16_t seq;
	uint64_t last_ms;
	pthread_mutex_t mtx;
} snowflake_t;

static inline uint64_t now_millis(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

int snowflake_init(snowflake_t *sf, uint16_t node_id)
{
	if (!sf)
		return EINVAL;
	if (node_id > SF_NODE_MAX)
		return ERANGE;

	sf->node_id = node_id;
	sf->seq = 0;
	sf->last_ms = 0;
	return pthread_mutex_init(&sf->mtx, NULL);
}

uint64_t snowflake_next(snowflake_t *sf)
{
	pthread_mutex_lock(&sf->mtx);
	uint64_t id = snowflake_gen_next(&sf->last_ms, sf->node_id, &sf->seq,
					 now_millis());
	pthread_mutex_unlock(&sf->mtx);
	return id;
}

#endif
