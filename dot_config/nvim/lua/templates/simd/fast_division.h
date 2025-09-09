#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
	__m256i multiplier;
	int shift;
} fast_divider;

inline fast_divider fast_divider_create(uint32_t divisor)
{
	fast_divider fd;
	uint64_t mul = (1ULL << 32) / divisor;
	fd.multiplier = _mm256_set1_epi32((int)mul);
	fd.shift = __builtin_ctz(divisor);
	return fd;
}

inline __m256i fast_divide(__m256i x, fast_divider fd)
{
	__m256i hi = _mm256_srli_epi64(_mm256_mul_epu32(x, fd.multiplier), 32);
	__m256i lo = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), fd.multiplier);
	__m256i result = _mm256_blend_epi32(hi, lo, 0xAA);
	return _mm256_srli_epi32(result, fd.shift);
}

 void divide_vector_simd(int *dst, const int *src, size_t count,
			  uint32_t divisor)
{
	fast_divider fd = fast_divider_create(divisor);
	size_t i;

	for (i = 0; i < count - (count % 8); i += 8) {
		__m256i x = _mm256_loadu_si256((__m256i *)&src[i]);
		__m256i result = fast_divide(x, fd);
		_mm256_storeu_si256((__m256i *)&dst[i], result);
	}

	for (; i < count; i++) {
		dst[i] = src[i] / divisor;
	}
}
