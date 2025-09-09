#ifndef SNOWFLAKE_ATOMIC_H
#define SNOWFLAKE_ATOMIC_H

#include <stdint.h>

#ifndef SNF_NODE_BITS
#define SNF_NODE_BITS 10
#endif
#ifndef SNF_SEQ_BITS
#define SNF_SEQ_BITS 12
#endif
#ifndef SNF_EPOCH_MILLIS
#define SNF_EPOCH_MILLIS 1700000000000ULL
#endif
enum {
	SNF_NODE_MAX = (1U << SNF_NODE_BITS) - 1,
	SNF_SEQ_MAX = (1U << SNF_SEQ_BITS) - 1
};

typedef struct {
	volatile uint64_t packed;
} snf_gen_t;

static inline uint64_t snf__pack(uint64_t ms, uint16_t seq)
{
	return (ms << SNF_SEQ_BITS) | seq;
}

static inline uint64_t snf__make_id(uint64_t ms, uint16_t node, uint16_t seq)
{
	return ((ms - SNF_EPOCH_MILLIS) << (SNF_NODE_BITS + SNF_SEQ_BITS)) |
	       ((uint64_t)node << SNF_SEQ_BITS) | seq;
}

#if defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 407
#define SNF_LOAD(ptr) __atomic_load_n((ptr), __ATOMIC_RELAXED)
#define SNF_CAS(ptr, old, neu)                                                \
	__atomic_compare_exchange_n((ptr), (old), (neu), 1, __ATOMIC_ACQ_REL, \
				    __ATOMIC_RELAXED)
#else /* GCC < 4.7 … fall back to __sync_* */
#define SNF_LOAD(ptr) __sync_val_compare_and_swap((ptr), 0, 0)
#define SNF_CAS(ptr, old, neu) \
	__sync_bool_compare_and_swap((ptr), *(old), (neu))
#endif

static inline uint64_t snf_next(snf_gen_t *st, uint16_t node_id,
				uint64_t now_ms)
{
	uint64_t old, last_ms, neu;
	uint16_t seq;
	for (;;) {
		old = SNF_LOAD(&st->packed);

		last_ms = old >> SNF_SEQ_BITS;
		seq = (uint16_t)(old & SNF_SEQ_MAX);

		if (now_ms < last_ms)
			now_ms = last_ms;

		if (now_ms == last_ms) {
			if (++seq > SNF_SEQ_MAX) {
				now_ms++;
				seq = 0;
			}
		} else {
			seq = 0;
		}

		neu = snf__pack(now_ms, seq);

		if (SNF_CAS(&st->packed, &old, neu))
			return snf__make_id(now_ms, node_id & SNF_NODE_MAX,
					    seq);
	}
}

#endif /* SNOWFLAKE_ATOMIC_H */

