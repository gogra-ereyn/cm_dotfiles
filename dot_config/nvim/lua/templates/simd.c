#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>

inline int horizontal_min_i32(const __m256i v) {
    __m256i min1 = _mm256_permute2x128_si256(v, v, 1);
    __m256i min2 = _mm256_min_epi32(v, min1);
    __m256i min3 = _mm256_shuffle_epi32(min2, _MM_SHUFFLE(1,0,3,2));
    __m256i min4 = _mm256_min_epi32(min2, min3);
    __m256i min5 = _mm256_shuffle_epi32(min4, _MM_SHUFFLE(0,0,0,1));
    return _mm256_cvtsi256_si32(_mm256_min_epi32(min4, min5));
}

inline __m256i abs_epi32_fast(__m256i x) {
    __m256i mask = _mm256_srai_epi32(x, 31);
    return _mm256_xor_si256(_mm256_add_epi32(x, mask), mask);
}

inline __m256i select_epi32(__m256i mask, __m256i a, __m256i b) {
    return _mm256_blendv_epi8(b, a, mask);
}

inline __m256i find_char_avx2(const char* data, char target) {
    __m256i target_vec = _mm256_set1_epi8(target);
    __m256i data_vec = _mm256_loadu_si256((__m256i*)data);
    return _mm256_cmpeq_epi8(data_vec, target_vec);
}

void interleaved_process(float* dst, const float* src1, const float* src2, size_t count) {
    size_t i;
    for (i = 0; i < count; i += 8) {
        __m256 a = _mm256_load_ps(&src1[i]);
        __m256 b = _mm256_load_ps(&src2[i]);
        __m256 sum = _mm256_add_ps(a, b);
        __m256 product = _mm256_mul_ps(a, b);
        __m256 result = _mm256_fmadd_ps(sum, product, _mm256_set1_ps(1.0f));
        _mm256_store_ps(&dst[i], result);
    }
}

inline __m256i masked_load(const int* ptr, __m256i mask) {
    return _mm256_maskload_epi32(ptr, mask);
}

typedef struct {
    uint8_t table[256];
} SmallLUT;

inline __m256i lut_lookup_avx2(__m256i indices, const SmallLUT* lut) {
    __m256i result;
    uint8_t* result_ptr = (uint8_t*)&result;
    const uint8_t* indices_ptr = (const uint8_t*)&indices;
    size_t i;
    for (i = 0; i < 32; i++) {
        result_ptr[i] = lut->table[indices_ptr[i]];
    }
    return result;
}

typedef struct {
    __m256i prev_values;
    __m256i threshold;
} StreamDetector;

inline __m256i detect_edges(__m256i current, StreamDetector* detector) {
    __m256i diff = _mm256_sub_epi32(current, detector->prev_values);
    __m256i abs_diff = abs_epi32_fast(diff);
    __m256i mask = _mm256_cmpgt_epi32(abs_diff, detector->threshold);
    detector->prev_values = current;
    return mask;
}

typedef struct {
    int bins[8][32];
} SIMDHistogram;

inline void histogram_accumulate(__m256i values, SIMDHistogram* hist) {
    size_t lane;
    for (lane = 0; lane < 8; lane++) {
        int idx = _mm256_extract_epi32(values, lane);
        hist->bins[lane][idx & 31]++;
    }
}

inline __m256i histogram_merge(const SIMDHistogram* hist) {
    __m256i result = _mm256_setzero_si256();
    size_t lane, bin;
    for (lane = 0; lane < 8; lane++) {
        for (bin = 0; bin < 32; bin++) {
            result = _mm256_add_epi32(result,
                _mm256_set1_epi32(hist->bins[lane][bin]));
        }
    }
    return result;
}




