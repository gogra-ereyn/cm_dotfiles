#include "massert.h"
#include "solve.h"
#define ALEN(x) (sizeof(x) / sizeof(*x))

int main(int argc, char **argv)
{
	int count = 0;
	{
		int nums[] = { 1, 1, 1, 2, 2, 3 };
		int expected[] = { 1, 2 };
		int outlen = 0;
		int *ans;
		ans = topKFrequent(nums, ALEN(nums), 2, &outlen);
		assert_eq(ALEN(expected), 2, "mismatched lengths");

		for (int i = 0; i < outlen; ++i) {
			assert_eq(expected[i], ans[i], "values not equal at index=%d", i);
		}
		dprintf(2, "case #%d PASSED\n", ++count);
	}
	dprintf(2, "ALL %d PASSED\n", count);
	return 0;
}
