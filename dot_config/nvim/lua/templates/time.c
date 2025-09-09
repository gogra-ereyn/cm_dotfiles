#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

uint64_t now_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)(ts.tv_sec * 1000) + (ts.tv_nsec / 1000);
}

uint64_t now_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000) + (uint64_t)ts.tv_nsec);
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
