#include <stdio.h>

void print_bits(int num)
{
	for (int bit = 31; bit >= 0; bit--) {
		printf("%d", (num >> bit) & 1);
		if (bit % 4 == 0)
			printf(" ");
	}
	printf("(%d)\n");
}

