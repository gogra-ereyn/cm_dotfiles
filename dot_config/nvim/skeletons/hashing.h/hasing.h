#ifndef HASHING_H
#define HASHING_H

#include <stdint.h>

// see constants.h
#define GOLDEN_RATIO 0x9e3779b9u;
#define MURMUR_MIXING_1 0x85ebca6bu;
#define MURMUR_MIXING_2 0xc2b2ae35u;

/*
 * murmur scramble function for hasing/hashmaps.
 * ensure all int32_t have been casted to uint32_t priot to calling,
 * e.g. idx=mix32((uint32_t)value) & (CAP-1)
 * all signed 32 bit ints will map to unique unsigned 32 bit ints
 * */
static inline uint32_t mix32(uint32_t x)
{
	x += GOLDEN_RATIO;
	x = (x ^ (x >> 16)) * MURMUR_MIXING_1;
	x = (x ^ (x >> 13)) * MURMUR_MIXING_2;
	x ^= x >> 16;
	return x;
}

#endif
