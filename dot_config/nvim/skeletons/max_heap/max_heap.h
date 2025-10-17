#include <limits.h>
static inline void sift_down(int *heap, int n, int i)
{
	int l, r, m, key;
	key = heap[i];
	for (;;) {
		l = i << 1;
		r = l + 1;
		if (l > n)
			break;
		m = l;
		if (r <= n && heap[r] > heap[l])
			m = r;
		if (!(heap[m] > key))
			break;
		heap[i] = heap[m];
		i = m;
	}
	heap[i] = key;
}

// assumes INT_MIN reserved
static inline void sift_up(int *heap, int i)
{
	int key, p;
	key = heap[i];
	for (;;) {
		p = i >> 1;
		if (!(key > heap[p]))
			break;
		heap[i] = heap[p]; // demoted
		i = p;
	}
	heap[i] = key;
}

static inline int findKthLargest(int *nums, int len, int k)
{
#define MAX (1 << 17)

	if (!len)
		return 0;

	int i = 0, count = 0;
	int heap[MAX] = { 0 };
	heap[0] = INT_MAX;
	heap[1] = nums[i++];

	count = 1;

	// build heap: sift up
	for (; i < len; ++i) {
		heap[++count] = nums[i];
		sift_up(heap, count);
	}

	for (i = 0; --k;) {
		heap[1] = heap[count--];
		sift_down(heap, count, 1);
	}

	return heap[1];
}

