#include <stdint.h>
#include <assert.h>

#define READ_PERMISSION (1 << 0) // 0001
#define WRITE_PERMISSION (1 << 1) // 0010
#define EXECUTE_PERMISSION (1 << 2) // 0100

char grant_rw()
{
	return READ_PERMISSION | WRITE_PERMISSION;
}

static inline uint32_t __be32_to_le(uint32_t val)
{
	uint32_t c1, c2, c3, c4;

	c1 = (val >> 24) & 0xff;
	c2 = (val >> 16) & 0xff;
	c3 = (val >> 8) & 0xff;
	c4 = val & 0xff;

	return c1 | c2 << 8 | c3 << 16 | c4 << 24;
}

static inline uint64_t __be64_to_le(uint64_t val)
{
	uint64_t c1, c2, c3, c4, c5, c6, c7, c8;

	c1 = (val >> 56) & 0xff;
	c2 = (val >> 48) & 0xff;
	c3 = (val >> 40) & 0xff;
	c4 = (val >> 32) & 0xff;
	c5 = (val >> 24) & 0xff;
	c6 = (val >> 16) & 0xff;
	c7 = (val >> 8) & 0xff;
	c8 = val & 0xff;

	return c1 | c2 << 8 | c3 << 16 | c4 << 24 | c5 << 32 | c6 << 40 |
	       c7 << 48 | c8 << 56;
}

static inline uint16_t swap_bytes_u16v2(uint16_t value)
{
	return ((value << 8) & 0xFF00) | ((value >> 8) & 0x00FF);
}

static inline uint16_t swap_bytes_u16(uint16_t x)
{
	return (x << 8) | (x >> 8);
}

void test_swap_bytes_16()
{
	assert(swap_bytes_u16(0x0000) == 0x0000);
	assert(swap_bytes_u16(0x1234) == 0x3412);
	assert(swap_bytes_u16(0xABCD) == 0xCDAB);
	assert(swap_bytes_u16(0xFF00) == 0x00FF);
	assert(swap_bytes_u16(0x00FF) == 0xFF00);
	assert(swap_bytes_u16(0xFFFF) == 0xFFFF);
	assert(swap_bytes_u16(0x0001) == 0x0100);
	assert(swap_bytes_u16(0x8000) == 0x0080);
}

static inline uint16_t to_big_endian(uint16_t value)
{
	return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
}

static inline uint32_t swap_bytes_32(uint32_t x)
{
	return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) |
	       ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24);
}

static inline uint64_t swap_bytes_64(uint64_t x)
{
	return ((x & 0xFF00000000000000ULL) >> 56) |
	       ((x & 0x00FF000000000000ULL) >> 40) |
	       ((x & 0x0000FF0000000000ULL) >> 24) |
	       ((x & 0x000000FF00000000ULL) >> 8) |
	       ((x & 0x00000000FF000000ULL) << 8) |
	       ((x & 0x0000000000FF0000ULL) << 24) |
	       ((x & 0x000000000000FF00ULL) << 40) |
	       ((x & 0x00000000000000FFULL) << 56);
}

static inline char toggle_case(char val)
{
	return val ^ 32;
}
static inline char to_upper(char val)
{
	return val & ~32;
}
static inline char to_lower(char val)
{
	return val | 32;
}
static inline int is_alphanum(char c)
{
	return ((c | 32) >= 'a' && (c | 32) <= 'z') || ((c >= '0' && c <= '9'));
}

static inline int safe_average(int a, int b)
{
	return (a & b) + ((a ^ b) >> 1);
}

static inline int fast_abs(int n)
{
	int mask = n >> 31;
	return (n + mask) ^ mask;
}

static inline int fast_mod(int n, int x)
{
	return n & x;
}

int did_add_wrap(int a, int b, int *suc)
{
	int result = a + b;
	*suc = !(((a ^ result) & (b ^ result)) < 0);
	return result;
}

static inline void swap_ints(int *a, int *b)
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

#define did_add_wrap(a, b, result_ptr) \
	(*(result_ptr) = (a) + (b),    \
	 (((a) ^ *(result_ptr)) & ((b) ^ *(result_ptr))) > 0)

// runtime
uint32_t next_pow2_gt(uint32_t n)
{
	if (n == 0)
		return 1;
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n + 1;
}

uint64_t next_pow2_gt_u64(uint64_t n)
{
	if (n == 0)
		return 1;
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;
	return n + 1;
}

int signless_flip(int val)
{
	int sign = -1;
	val ^= sign;
	val -= sign;
	return val;
}

int zeroing_no_branching(int val)
{
	return val ^= val;
}

int have_opposite_signs(int a, int b)
{
	return ((a ^ b) < 0);
}

int example_toggle_xor()
{
	int cur = 1, a = 2, b = 3;
	// if cur == a, then it becomes b; if cur == b , then it becomes a
	cur ^= a ^ b;
	return cur;
}


int remove_lowest_set_bit(int x)
{
	return x &= x - 1;
}

// comptime, gcc
#define NEXT_POW2(n) (1U << (32 - __builtin_clz((n) - 1)))
#define NEXT_POW2_U64(n) (1ULL << (64 - __builtin_clzll((n) - 1)))
