#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

static const char b64_enc_tbl[64] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const uint8_t b64_dec_tbl[256] = {
	[0 ... 255] = 0xFF, ['A'] = 0,	['B'] = 1,  ['C'] = 2,	['D'] = 3,
	['E'] = 4,	    ['F'] = 5,	['G'] = 6,  ['H'] = 7,	['I'] = 8,
	['J'] = 9,	    ['K'] = 10, ['L'] = 11, ['M'] = 12, ['N'] = 13,
	['O'] = 14,	    ['P'] = 15, ['Q'] = 16, ['R'] = 17, ['S'] = 18,
	['T'] = 19,	    ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
	['Y'] = 24,	    ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28,
	['d'] = 29,	    ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33,
	['i'] = 34,	    ['j'] = 35, ['k'] = 36, ['l'] = 37, ['m'] = 38,
	['n'] = 39,	    ['o'] = 40, ['p'] = 41, ['q'] = 42, ['r'] = 43,
	['s'] = 44,	    ['t'] = 45, ['u'] = 46, ['v'] = 47, ['w'] = 48,
	['x'] = 49,	    ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53,
	['2'] = 54,	    ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58,
	['7'] = 59,	    ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63,
	['='] = 0xFE
};

static const char hex_enc_tbl[16] = "0123456789abcdef";

static const uint8_t hex_dec_tbl[256] = {
	[0 ... 255] = 0xFF, ['0'] = 0,	['1'] = 1,  ['2'] = 2,	['3'] = 3,
	['4'] = 4,	    ['5'] = 5,	['6'] = 6,  ['7'] = 7,	['8'] = 8,
	['9'] = 9,	    ['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13,
	['E'] = 14,	    ['F'] = 15, ['a'] = 10, ['b'] = 11, ['c'] = 12,
	['d'] = 13,	    ['e'] = 14, ['f'] = 15
};

static inline size_t hex_encoded_length(size_t n)
{
	return n << 1;
}
static inline size_t b64_encoded_length(size_t n)
{
	return ((n + 2) / 3) << 2;
}

static inline size_t hex_encode(const uint8_t *in, size_t n, char *out)
{
	for (size_t i = 0; i < n; i++) {
		uint8_t b = in[i];
		*out++ = hex_enc_tbl[b >> 4];
		*out++ = hex_enc_tbl[b & 15];
	}
	return n << 1;
}

static inline ssize_t hex_decode(const char *in, size_t n, uint8_t *out)
{
	if (n & 1)
		return -1;
	for (size_t i = 0; i < n; i += 2) {
		uint8_t hi = hex_dec_tbl[(uint8_t)in[i]],
			lo = hex_dec_tbl[(uint8_t)in[i + 1]];
		if ((hi | lo) == 0xFF)
			return -1;
		*out++ = (hi << 4) | lo;
	}
	return n >> 1;
}

static inline size_t base64_encode(const uint8_t *in, size_t n, char *out)
{
	size_t i = 0, j = 0;
	for (; i + 2 < n; i += 3) {
		uint32_t v = (in[i] << 16) | (in[i + 1] << 8) | in[i + 2];
		out[j++] = b64_enc_tbl[(v >> 18) & 63];
		out[j++] = b64_enc_tbl[(v >> 12) & 63];
		out[j++] = b64_enc_tbl[(v >> 6) & 63];
		out[j++] = b64_enc_tbl[v & 63];
	}
	size_t rem = n - i;
	if (rem) {
		uint32_t v = in[i] << 16;
		if (rem == 2)
			v |= in[i + 1] << 8;
		out[j + 3] = '=';
		out[j + 2] = rem == 2 ? b64_enc_tbl[(v >> 6) & 63] : '=';
		out[j + 1] = b64_enc_tbl[(v >> 12) & 63];
		out[j] = b64_enc_tbl[(v >> 18) & 63];
		j += 4;
	}
	return j;
}

static inline ssize_t base64_decode(const char *in, size_t n, uint8_t *out)
{
	if (n & 3)
		return -1;
	size_t i = 0, j = 0;
	while (i < n) {
		uint8_t a = b64_dec_tbl[(uint8_t)in[i++]],
			b = b64_dec_tbl[(uint8_t)in[i++]],
			c = b64_dec_tbl[(uint8_t)in[i++]],
			d = b64_dec_tbl[(uint8_t)in[i++]];
		if ((a | b | c | d) == 0xFF)
			return -1;
		uint32_t v = (a << 18) | (b << 12) | ((c & 63) << 6) | (d & 63);
		out[j++] = v >> 16;
		if (c != 0xFE) {
			out[j++] = (v >> 8) & 255;
			if (d != 0xFE)
				out[j++] = v & 255;
			else if (in[i - 1] != '=')
				return -1;
		} else if (in[i - 2] != '=')
			return -1;
		if ((c == 0xFE && d != 0xFE) || (a == 0xFE || b == 0xFE))
			return -1;
		if (d == 0xFE && i != n)
			return -1;
	}
	return j;
}
