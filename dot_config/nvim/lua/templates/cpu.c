#include <stdio.h>

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <cpuid.h>
#endif

void get_cpuid(int info[4], int type)
{
#if defined(_MSC_VER)
	__cpuid(info, type);
#elif defined(__GNUC__)
	__cpuid(type, info[0], info[1], info[2], info[3]);
#endif
}

int main()
{
	int info[4];
	int max_simd_width = 0;

	char vendor[13];
	get_cpuid(info, 0);
	((int *)vendor)[0] = info[1];
	((int *)vendor)[1] = info[3];
	((int *)vendor)[2] = info[2];
	vendor[12] = '\0';
	printf("CPU Vendor: %s\n", vendor);
	get_cpuid(info, 1);
	printf("\nSIMD Instructions Support:\n");
	printf("MMX (64-bit):     %s\n", (info[3] & (1 << 23)) ? "Yes" : "No");
	printf("SSE (128-bit):    %s\n", (info[3] & (1 << 25)) ? "Yes" : "No");
	printf("SSE2 (128-bit):   %s\n", (info[3] & (1 << 26)) ? "Yes" : "No");
	printf("SSE3 (128-bit):   %s\n", (info[2] & (1 << 0)) ? "Yes" : "No");
	printf("SSSE3 (128-bit):  %s\n", (info[2] & (1 << 9)) ? "Yes" : "No");
	printf("SSE4.1 (128-bit): %s\n", (info[2] & (1 << 19)) ? "Yes" : "No");
	printf("SSE4.2 (128-bit): %s\n", (info[2] & (1 << 20)) ? "Yes" : "No");

	int avx_supported = 0;
	if (info[2] & (1 << 28)) {
		unsigned long long xcr0;
#if defined(_MSC_VER)
#elif defined(__GNUC__)
		__asm__("xgetbv" : "=A"(xcr0) : "c"(0));
#endif
		avx_supported = (xcr0 & 6) == 6;
	}
	printf("AVX (256-bit):    %s\n", avx_supported ? "Yes" : "No");

	get_cpuid(info, 7);
	printf("AVX2 (256-bit):   %s\n", (info[1] & (1 << 5)) ? "Yes" : "No");
	printf("AVX-512 (512-bit):%s\n", (info[1] & (1 << 16)) ? "Yes" : "No");

	if (info[1] & (1 << 16))
		max_simd_width = 512;
	else if (avx_supported)
		max_simd_width = 256;
	else if (info[3] & (1 << 25))
		max_simd_width = 128;
	else if (info[3] & (1 << 23))
		max_simd_width = 64;
	printf("\nMaximum SIMD width: %d bits\n", max_simd_width);
	return 0;
}
