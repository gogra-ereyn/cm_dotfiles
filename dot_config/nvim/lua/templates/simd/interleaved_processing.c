#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>

void interleaved_process(float *dst, const float *src1, const float *src2,
			 size_t count)
{
	size_t i;
	for (i = 0; i < count; i += 8) {
		__m256 a = _mm256_load_ps(&src1[i]);
		__m256 b = _mm256_load_ps(&src2[i]);
		__m256 sum = _mm256_add_ps(a, b);
		__m256 product = _mm256_mul_ps(a, b);
		__m256 result =
			_mm256_fmadd_ps(sum, product, _mm256_set1_ps(1.0f));
		_mm256_store_ps(&dst[i], result);
	}
}

