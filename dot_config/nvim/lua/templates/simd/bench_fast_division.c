#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fast_division.h"


void divide_vector_scalar(int* dst, const int* src, size_t count, uint32_t divisor) {
    for (size_t i = 0; i < count; i++) {
        dst[i] = src[i] / divisor;
    }
}

void fill_random(int* arr, size_t count, int max_val) {
    for (size_t i = 0; i < count; i++) {
        arr[i] = rand() % max_val;
    }
}

int verify_results(const int* a, const int* b, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <array_size> <divisor> <max_value>\n", argv[0]);
        return 1;
    }

    size_t size = strtoull(argv[1], NULL, 10);
    uint32_t divisor = strtoul(argv[2], NULL, 10);
    int max_val = atoi(argv[3]);

    int* src = aligned_alloc(32, size * sizeof(int));
    int* dst_simd = aligned_alloc(32, size * sizeof(int));
    int* dst_scalar = aligned_alloc(32, size * sizeof(int));

    if (!src || !dst_simd || !dst_scalar) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    srand(42);
    fill_random(src, size, max_val);

    clock_t start = clock();
    divide_vector_simd(dst_simd, src, size, divisor);
    clock_t end = clock();
    double simd_time = (double)(end - start) / CLOCKS_PER_SEC;

    start = clock();
    divide_vector_scalar(dst_scalar, src, size, divisor);
    end = clock();
    double scalar_time = (double)(end - start) / CLOCKS_PER_SEC;

    int valid = verify_results(dst_simd, dst_scalar, size);

    printf("%zu,%u,%d,%f,%f,%d\n",
           size, divisor, max_val, simd_time, scalar_time, valid);

    free(src);
    free(dst_simd);
    free(dst_scalar);
    return 0;
}


