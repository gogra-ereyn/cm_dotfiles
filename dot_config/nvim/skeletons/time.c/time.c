#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// to print as eg millisecond precision: printf("%.3f s\n", now_ns_double_mono());
double now_ns_double_mono()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + ((double)ts.tv_nsec / 1e9);
}

// intentional precision loss
double now_ms_double_mono()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ((ts.tv_nsec / 1000000) / 1000.0);
}

uint64_t now_us_mono(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)(ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

uint64_t now_us(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)(ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

uint64_t now_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)(ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

uint64_t now_ms_mono(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)(ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

uint64_t now_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000) + (uint64_t)ts.tv_nsec;
}

// specfic stride
uint64_t get_max(uint64_t *times, unsigned count, unsigned stride)
{
	uint64_t max = 0;
	while (count--) {
		if (*times > max)
			max = *times;
		times = (uint64_t *)((void *)times + stride);
	}
	return max;
}
// generic stride
void process_data(void *data, size_t count, size_t stride,
		  void (*process_fn)(void *))
{
	char *ptr = (char *)data;
	for (size_t i = 0; i < count; i++) {
		process_fn(ptr);
		ptr += stride; // Move to the next element using stride
	}
}

static inline uint64_t ts_conv_f64_ms_to_u64_ms(double ts)
{
	return (uint64_t)((ts * 1000.0) + 0.5);
}

static inline uint64_t ts_conv_u64_ms_to_f64_ms(uint64_t ts)
{
	return ts * 0.001;
}
