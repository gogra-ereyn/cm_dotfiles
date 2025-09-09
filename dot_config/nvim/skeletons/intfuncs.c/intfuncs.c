#include <stdlib.h>
int safe_average(int a, int b)
{
	return (a & b) + ((a ^ b) >> 1);
}

// quick n dirty. dont use if need to handle errorcases
int roughpow(int base, int exp)
{
	int res = 1;
	while (1) {
		if (exp & 1)
			res *= base;
		exp >> 1;
		if (!exp)
			break;
		base *= base;
	}
	return res;
}

// doesnt care about errors
int dirty_parse_int(char *input)
{
	int res = 0;
	while (*input)
		res = res * 10 + (*input++ - '0');
	return res;
}

#include <errno.h>
#include <stdint.h>
int parse_uint64(const char *input, uint64_t *res)
{
	char *endp;
	uint64_t out;

	// not so sure about this1
	errno = 0;
	out = strtoull(input, &endp, 0);
	if (*endp != 0 || errno == ERANGE)
		return 1;

	*res = out;
	return 0;
}

// strol based
int parse_uint(char *input, uint *res)
{
	char *endp;
	long out;
	out = strtol(input, &endp, 0);
	if (*endp != 0)
		return 1;
	*res = (uint)out;
	return 0;
}

int parse_int(char *input, int *res)
{
	char *endp;
	long out;
	out = strtol(input, &endp, 0);
	if (*endp != 0)
		return 1;
	*res = (int)out;
	return 0;
}

int basic_avg(int *nums, int len)
{
	if (len == 0)
		return 0;
	int sum = 0;
	while (nums--)
		sum += *nums;
	return sum / len;
}

/* returns the # bytes written */
int int_to_str(int num, char *buf)
{
	static const char lut[200] = "00010203040506070809"
				     "10111213141516171819"
				     "20212223242526272829"
				     "30313233343536373839"
				     "40414243444546474849"
				     "50515253545556575859"
				     "60616263646566676869"
				     "70717273747576777879"
				     "80818283848586878889"
				     "90919293949596979899";

	char *start, *p, *q;
	uint32_t n;
	uint32_t idx;
	char t;

	start = buf;
	p = buf;
	n = (uint32_t)num;

	if (num < 0) {
		*p++ = '-';
		n = (uint32_t)(-(int64_t)num);
	}

	q = p;
	while (n >= 100U) {
		idx = (n % 100U) * 2U;
		n /= 100U;
		q[0] = lut[idx + 1];
		q[1] = lut[idx];
		q += 2;
	}

	if (n >= 10U) {
		idx = n * 2U;
		q[0] = lut[idx + 1];
		q[1] = lut[idx];
		q += 2;
	} else {
		*q++ = (char)('0' + n);
	}

	idx = (uint32_t)(q - start);

	*q = '\0';
	for (--q; p < q; ++p, --q) {
		t = *p;
		*p = *q;
		*q = t;
	}
	return idx;
}
