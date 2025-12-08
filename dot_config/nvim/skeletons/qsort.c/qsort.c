#include <string.h>
#include <stdlib.h>



/* for sorting strings by length */
int compare_strlen(const void *a, const void *b)
{
	const char *str_a = *(const char **)a;
	const char *str_b = *(const char **)b;
	size_t len_a = strlen(str_a);
	size_t len_b = strlen(str_b);

	if (len_a < len_b)
		return -1;
	if (len_a > len_b)
		return 1;
	return 0;
}

