#include <stdint.h>

#define READ_PERMISSION (1 << 0) // 0001
#define WRITE_PERMISSION (1 << 1) // 0010
#define EXECUTE_PERMISSION (1 << 2) // 0100

char grant_rw()
{
	return READ_PERMISSION | WRITE_PERMISSION;
}

unsigned int swap_bytes_16(uint16_t x)
{
	return (x << 8) | (x >> 8);
}

uint32_t swap_bytes_32(uint32_t x)
{
	return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) |
	       ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24);
}

uint64_t swap_bytes_64(uint64_t x)
{
    return ((x & 0xFF00000000000000ULL) >> 56) |
           ((x & 0x00FF000000000000ULL) >> 40) |
           ((x & 0x0000FF0000000000ULL) >> 24) |
           ((x & 0x000000FF00000000ULL) >> 8)  |
           ((x & 0x00000000FF000000ULL) << 8)  |
           ((x & 0x0000000000FF0000ULL) << 24) |
           ((x & 0x000000000000FF00ULL) << 40) |
           ((x & 0x00000000000000FFULL) << 56);
}


char toggle_case(char val)
{
	return val ^ 32;
}
char to_upper(char val)
{
	return val & ~32;
}
char to_lower(char val)
{
	return val | 32;
}

int safe_average(int a, int b)
{
	return (a & b) + ((a ^ b) >> 1);
}

int fast_abs(int n)
{
	int mask = n >> 31;
	return (n + mask) ^ mask;
}

int fast_mod(int n, int x)
{
	return n & x;
}

int did_add_wrap(int a, int b, int *suc)
{
	int result = a + b;
	*suc=!(((a ^ result) & (b ^ result)) < 0);
	return result;
}



#define did_add_wrap(a, b, result_ptr) \
    (*(result_ptr) = (a) + (b), \
     (((a) ^ *(result_ptr)) & ((b) ^ *(result_ptr))) > 0)
