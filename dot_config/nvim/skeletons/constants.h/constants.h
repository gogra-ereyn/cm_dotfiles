#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <stdint.h>

/* fractional part of the golden ratio × 2³².
 * commonly used as a multiplicative constant to spread out integer keys (appears in knuth’s multiplicative hashing). */
uint32_t golden_ratio = 0x9e3779b9u;

/* a MurmurHash3 mixing constant chosen empirically for good avalanche behavior on 32-bit multiplies. */
uint32_t murmur_mixing_1 = 0x85ebca6bu;

/* another MurmurHash3 mixing constant, complementary to the previous one.
 * together with the above they ensure that each output bit depends on all input bits in a nonlinear way. */
uint32_t murmur_mixing_2 = 0xc2b2ae35u;

#endif
