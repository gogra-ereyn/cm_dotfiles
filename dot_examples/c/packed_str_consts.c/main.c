#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void print_escaped(const char *str)
{
	size_t i;

	for (i = 0; str[i] != '\0'; i++) {
		switch (str[i]) {
		case '\r':
			dprintf(1, "\\r");
			break;
		case '\n':
			dprintf(1, "\\n");
			break;
		case '\t':
			dprintf(1, "\\t");
			break;
		case '\\':
			dprintf(1, "\\\\");
			break;
		case '"':
			dprintf(1, "\\\"");
			break;
		default:
			dprintf(1, "%c", str[i]);
			break;
		}
	}
}

static void print_packed_constant4(const char *str)
{
	size_t len;
	uint32_t packed;
	size_t i;

	len = strlen(str);
	if (len > 4) {
		dprintf(1, "/* string too long (>4 chars): \"");
		print_escaped(str);
		dprintf(1, "\" */\n");
		return;
	}

	packed = 0;
	for (i = 0; i < len; i++) {
		packed |= ((uint32_t)(unsigned char)str[i]) << (i << 3);
	}

	dprintf(1, "*(uint32_t*)buf = 0x%08xu; /* \"", packed);
	print_escaped(str);
	dprintf(1, "\" */\n");
}

static void print_packed_constant8(const char *str)
{
	size_t len;
	uint64_t packed;
	size_t i;

	len = strlen(str);
	if (len > 8) {
		dprintf(1, "/* String too long (>8 chars): \"");
		print_escaped(str);
		dprintf(1, "\" */\n");
		return;
	}

	packed = 0;
	for (i = 0; i < len; i++) {
		packed |= ((uint64_t)(unsigned char)str[i]) << (i << 3);
	}

	dprintf(1, "*(uint64_t*)buf = 0x%016lxull; /* \"", packed);
	print_escaped(str);
	dprintf(1, "\" */\n");
}

/* utils */

static void print_from_packed64(uint64_t packed)
{
	size_t i;
	unsigned char c;

	dprintf(2, "\"");
	for (i = 0; i < 8; i++) {
		c = (packed >> (i * 8)) & 0xFF;
		if (c == 0)
			break;

		switch (c) {
		case '\r':
			dprintf(2, "\\r");
			break;
		case '\n':
			dprintf(2, "\\n");
			break;
		case '\t':
			dprintf(2, "\\t");
			break;
		case '\\':
			dprintf(2, "\\\\");
			break;
		case '"':
			dprintf(2, "\\\"");
			break;
		default:
			dprintf(2, "%c", c);
			break;
		}
	}
	dprintf(2, "\"\n");
}

static void print_from_packed32(uint32_t packed)
{
	size_t i;
	unsigned char c;

	dprintf(2, "\"");
	for (i = 0; i < 4; i++) {
		c = (packed >> (i << 3)) & 0xFF;
		if (c == 0)
			break;

		switch (c) {
		case '\r':
			dprintf(2, "\\r");
			break;
		case '\n':
			dprintf(2, "\\n");
			break;
		case '\t':
			dprintf(2, "\\t");
			break;
		case '\\':
			dprintf(2, "\\\\");
			break;
		case '"':
			dprintf(2, "\\\"");
			break;
		default:
			dprintf(2, "%c", c);
			break;
		}
	}
	dprintf(2, "\"\n");
}

/*end utils*/

void print_debug()
{
	print_from_packed64(0x002f2f3a70747468ULL);
	print_from_packed64(0x312e312f50545448ull);

	print_from_packed32(0x20544547U); /* "GET " */
	print_from_packed32(0x0a0d0a0du);
}

int main(void)
{
	print_debug();
	print_packed_constant8("http://");
	print_packed_constant8("\r\n");
	print_packed_constant8("HTTP/1.1");
	print_packed_constant8(": ");
	print_packed_constant8("GET ");

	print_packed_constant4("GET ");
	print_packed_constant4("POST");
	print_packed_constant4("PUT ");
	print_packed_constant4("HEAD");
	print_packed_constant4("HTTP");
	print_packed_constant4("\r\n\r\n"); /* end of headers */
	print_packed_constant4("200 "); /* status code prefix */
	print_packed_constant4("404 ");
	print_packed_constant4("500 ");
	print_packed_constant4("Host");
	print_packed_constant4("User");
	print_packed_constant4("text");
	print_packed_constant4("html");
	print_packed_constant4("json");
	print_packed_constant4("true");
	print_packed_constant4("null");
	print_packed_constant4("    "); /* four spaces for indentation xd */
	return 0;
}
