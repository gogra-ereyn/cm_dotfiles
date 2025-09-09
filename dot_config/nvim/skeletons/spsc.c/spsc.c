#include <stdint.h>

#define RING_ORDER 12
#define CAP (1 << RING_ORDER)
#define MASK (CAP - 1)

struct ring {
	uint32_t head;
	uint32_t tail;
	uint32_t mask;
	uint32_t len;
	uint8_t *buf;
};

// just to give an example
static inline int next_idx(uint32_t i)
{
	return (i + 1) & MASK;
}

static inline int push(struct ring *r, uint32_t v)
{
	uint32_t h = r->head, t = r->tail;
	if (h - t == r->mask + 1)
		return -1;
	r->buf[h & r->mask] = v;
	__atomic_store_n(&r->head, h + 1, __ATOMIC_RELEASE);
	return 0;
}

static inline int pop(struct ring *r, uint32_t *o)
{
	uint32_t t = r->tail;
	if (t == __atomic_load_n(&r->head, __ATOMIC_ACQUIRE))
		return -1;
	*o = r->buf[t & r->mask];
	r->tail = t + 1;
	return 0;
}
