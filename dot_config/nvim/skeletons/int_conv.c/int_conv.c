#include "gogra_utils.h"

// clang-format off
static const char pairs[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
    '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
    '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
    '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
    '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
    '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
    '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
    '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
    '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};
// clang-format on

char *gga_to_str_u32(uint32_t value, char *end)
{
	char *p;
	uint32_t q, r;
	p = end;
	*p = 0;

	while (value >= 100) {
		q = value / 100;
		r = value % 100;
		p -= 2;
		*(uint16_t *)p = *(uint16_t *)&pairs[r << 1];
		value = q;
	}

	if (value >= 10) {
		p -= 2;
		*(uint16_t *)p = *(uint16_t *)&pairs[value << 1];
	} else {
		*--p = '0' + value;
	}

	return p;
}

char *gga_to_str_i32(int32_t value, char *buf_end)
{
	char *p;
	uint32_t uvalue;

	if (value < 0) {
		uvalue = (uint32_t)(-value);
		p = gga_to_str_u32(uvalue, buf_end);
		p--;
		*p = '-';
		return p;
	}

	return gga_to_str_u32((uint32_t)value, buf_end);
}

char *gga_to_str_u64(uint64_t value, char *buf_end)
{
	char *p;
	uint64_t q, r;

	p = buf_end;
	*p = '\0';

	while (value >= 100) {
		q = value / 100;
		r = value % 100;
		p -= 2;
		*(uint16_t *)p = *(uint16_t *)&pairs[r << 1];
		value = q;
	}

	if (value >= 10) {
		p -= 2;
		*(uint16_t *)p = *(uint16_t *)&pairs[value << 1];
	} else {
		p--;
		*p = '0' + value;
	}

	return p;
}

char *gga_to_str_i64(int64_t value, char *buf_end)
{
	char *p;
	uint64_t uvalue;
	if (value < 0) {
		uvalue = (uint64_t)(-value);
		p = gga_to_str_u64(uvalue, buf_end);
		p--;
		*p = '-';
		return p;
	}
	return gga_to_str_u64((uint64_t)value, buf_end);
}
