

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t json_escape_buffer_size(const char *input, size_t input_len)
{
	if (!input)
		return 1;
	size_t required_size = 1;
	for (size_t i = 0; i < input_len; i++) {
		char c = input[i];

		switch (c) {
		case '"':
		case '\\':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			required_size += 2;
			break;
		default:
			if (c >= 0 && c <= 0x1F) {
				required_size += 6;
			} else {
				required_size += 1;
			}
			break;
		}
	}
	return required_size;
}

int json_escape_to_buffer(const char *input, size_t input_len, char *output,
			  size_t output_size)
{
	if (!input || !output || output_size == 0) {
		return -1;
	}

	size_t out_pos = 0;
	for (size_t i = 0; i < input_len; i++) {
		char c = input[i];

		switch (c) {
		case '"':
			if (out_pos + 2 >= output_size)
				return -1;
			output[out_pos++] = '\\';
			output[out_pos++] = '"';
			break;
		case '\\':
			if (out_pos + 2 >= output_size)
				return -1;
			output[out_pos++] = '\\';
			output[out_pos++] = '\\';
			break;
		case '\b':
			if (out_pos + 2 >= output_size)
				return -1;
			output[out_pos++] = '\\';
			output[out_pos++] = 'b';
			break;
		case '\f':
			if (out_pos + 2 >= output_size)
				return -1;
			output[out_pos++] = '\\';
			output[out_pos++] = 'f';
			break;
		case '\n':
			if (out_pos + 2 >= output_size)
				return -1;
			output[out_pos++] = '\\';
			output[out_pos++] = 'n';
			break;
		case '\r':
			if (out_pos + 2 >= output_size)
				return -1;
			output[out_pos++] = '\\';
			output[out_pos++] = 'r';
			break;
		case '\t':
			if (out_pos + 2 >= output_size)
				return -1;
			output[out_pos++] = '\\';
			output[out_pos++] = 't';
			break;
		default:
			if (c >= 0 && c <= 0x1F) {
				if (out_pos + 6 >= output_size)
					return -1;
				snprintf(&output[out_pos], 7, "\\u%04x",
					 (unsigned char)c);
				out_pos += 6;
			} else {
				if (out_pos + 1 >= output_size)
					return -1;
				output[out_pos++] = c;
			}
			break;
		}
	}

	if (out_pos >= output_size)
		return -1;
	output[out_pos] = '\0';
	return (int)out_pos;
}

int json_escape_print(const char *input, size_t input_len)
{
	if (!input)
		return 0;

	int chars_printed = 0;
	for (size_t i = 0; i < input_len; i++) {
		char c = input[i];

		switch (c) {
		case '"':
			chars_printed += printf("\\\"");
			break;
		case '\\':
			chars_printed += printf("\\\\");
			break;
		case '\b':
			chars_printed += printf("\\b");
			break;
		case '\f':
			chars_printed += printf("\\f");
			break;
		case '\n':
			chars_printed += printf("\\n");
			break;
		case '\r':
			chars_printed += printf("\\r");
			break;
		case '\t':
			chars_printed += printf("\\t");
			break;
		default:
			if (c >= 0 && c <= 0x1F) {
				chars_printed +=
					printf("\\u%04x", (unsigned char)c);
			} else {
				chars_printed += printf("%c", c);
			}
			break;
		}
	}

	return chars_printed;
}

// demo
int main()
{
	const char *test_strings[] = { "Hello World",
				       "Hello \"World\"",
				       "Line 1\nLine 2",
				       "Tab\tSeparated",
				       "Backslash\\Path",
				       "Control\x01Character",
				       NULL };

	printf("Testing JSON string escaping:\n\n");
	for (int i = 0; test_strings[i] != NULL; i++) {
		size_t input_len = strlen(test_strings[i]);
		printf("original: \"%s\"\n", test_strings[i]);
		size_t buffer_size =
			json_escape_buffer_size(test_strings[i], input_len);

		char *buffer;
		buffer = malloc(buffer_size);
		if (buffer) {
			int result = json_escape_to_buffer(test_strings[i],
							   input_len, buffer,
							   buffer_size);
			if (result >= 0) {
				printf("buffered: \"%s\"\n", buffer);
			} else {
				printf("buffered: ERROR - buffer too small\n");
			}
			free(buffer);
		}
		printf("Direct:   \"");
		json_escape_print(test_strings[i], input_len);
		printf("\"\n\n");
	}
	return 0;
}
