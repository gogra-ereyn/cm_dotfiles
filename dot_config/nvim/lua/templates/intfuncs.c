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
int parse_int(char *input)
{
	int res = 0;
	while (*input)
		res = res * 10 + (*input++ - '0');
	return res;
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
