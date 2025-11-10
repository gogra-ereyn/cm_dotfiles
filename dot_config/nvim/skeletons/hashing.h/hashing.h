
#ifndef HASHING_H
#define HASHING_H

#include <stdint.h>

// see constants.h
#define HASH_GOLDEN_RATIO 0x9e3779b9u
#define HASH_MURMUR_MIX1  0x85ebca6bu
#define HASH_MURMUR_MIX2  0xc2b2ae35u

/*
 * murmur scramble function for hasing/hashmaps.
 * ensure all int32_t have been casted to uint32_t priot to calling,
 * e.g. idx=mix32((uint32_t)value) & (CAP-1)
 * all signed 32 bit ints will map to unique unsigned 32 bit ints
 * */
static inline uint32_t mix32(uint32_t x)
{
	x += HASH_GOLDEN_RATIO;
	x = (x ^ (x >> 16)) * HASH_MURMUR_MIX1;
	x = (x ^ (x >> 13)) * HASH_MURMUR_MIX2;
	x ^= x >> 16;
	return x;
}

#endif



