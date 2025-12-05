#ifndef LOOKUP_H
#define LOOKUP_H

#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

#define FB 11400714819323198485ULL

#ifndef MAP_BITS
#define MAP_BITS 21
#endif

#define MCAP (1u << MAP_BITS)
#define MMASK (MCAP - 1)

typedef struct {
	uint64_t k;
	uint32_t v;
} map_slot_t;

typedef struct {
	map_slot_t slot[MCAP];
	uint32_t next_id;
} sf_map_t;

static inline uint64_t sf_hash(uint64_t k)
{
	return (k * FB) & MMASK;
}

static inline void sf_map_init(sf_map_t *m)
{
	memset(m, 0, sizeof *m);
}

static inline int sf_map_full(sf_map_t *m)
{
	return m->next_id >= MCAP;
}

static inline uint32_t sf_map_get(const sf_map_t *m, uint64_t key)
{
	size_t i = sf_hash(key);
	for (;;) {
		uint64_t kk = m->slot[i].k;
		if (kk == key)
			return m->slot[i].v;
		if (kk == 0)
			return UINT32_MAX;
		i = (i + 1) & MMASK;
	}
}

static inline uint32_t sf_map_get_or_create(sf_map_t *m, uint64_t key)
{
	size_t i = sf_hash(key);
	for (;;) {
		uint64_t kk = m->slot[i].k;
		if (kk == key)
			return m->slot[i].v;
		if (kk == 0) {
			if (sf_map_full(m))
				return UINT32_MAX;
			uint32_t id = m->next_id++;
			m->slot[i].v = id;
			m->slot[i].k = key;
			return id;
		}
		i = (i + 1) & MMASK;
	}
}

static inline int sf_map_put(sf_map_t *m, uint64_t key, uint32_t value)
{
	size_t i = sf_hash(key);
	for (;;) {
		uint64_t kk = m->slot[i].k;
		if (kk == key || kk == 0) {
			m->slot[i].v = value;
			if (kk == 0 && value >= m->next_id)
				m->next_id = value + 1;
			m->slot[i].k = key;
			return 1;
		}
		i = (i + 1) & MMASK;
	}
}

#endif
