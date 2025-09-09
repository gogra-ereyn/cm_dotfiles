#ifndef CIRC_FRAMED_UTILS_H
#define CIRC_FRAMED_UTILS_H

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))


struct buffer_state {
        unsigned read_head;
        unsigned write_head;
        unsigned size;
};



static inline int next_message(const uint8_t *buff, unsigned capacity, uint8_t *out)
{
        if (capacity <= 2)
                return 0;

}


static inline int copy_complete_messages(const uint8_t *buff, unsigned read_head,
			       unsigned write_head, unsigned size,
			       unsigned capacity, uint8_t *out)
{

	if ((capacity & (capacity - 1)) == 0) {
                return -1;
	}

	if (!buff || !out) {
		return -2;
	}

	unsigned bytes_available;
	if (write_head == read_head) {
		bytes_available = size > 0 ? capacity : 0;
	} else if (write_head > read_head) {
		bytes_available = write_head - read_head;
	} else {
		bytes_available = capacity - read_head + write_head;
	}

	unsigned curr_pos = read_head;
	unsigned remaining = bytes_available;
	unsigned contiguous_bytes = 0;

	while (remaining >= 2) {
		uint16_t msg_len;
		if (curr_pos + 1 < capacity) {
			msg_len = (buff[curr_pos] << 8) | buff[curr_pos + 1];
		} else {
			msg_len = (buff[curr_pos] << 8) | buff[0];
		}

		if (remaining < msg_len + 2) {
			break;
		}
		contiguous_bytes += msg_len + 2;
		curr_pos = (curr_pos + msg_len + 2) % capacity;
		remaining -= msg_len + 2;
	}
	if (contiguous_bytes == 0) {
		return 0;
	}
	unsigned first_chunk =
		(write_head > read_head) ?
			contiguous_bytes :
			MIN(contiguous_bytes, capacity - read_head);

	memcpy(out, buff + read_head, first_chunk);
	if (first_chunk < contiguous_bytes) {
		memcpy(out + first_chunk, buff, contiguous_bytes - first_chunk);
	}
	return contiguous_bytes;
}

static inline int copy_messages(const uint8_t *buff, unsigned capacity, struct buffer_state *state, uint8_t *out, unsigned out_capacity)
{
        return copy_complete_messages(buff, state->read_head, state->write_head, state->size, capacity, out);
}



static inline size_t circ_avail(size_t start, size_t end, size_t size)
{
	return (end >= start) ? (end - start) : (size - start + end);
}

static inline size_t circ_space(size_t start, size_t end, size_t size)
{
	return size - 1 - circ_avail(start, end, size);
}

static inline size_t framed_io_read(int fd, uint8_t *buffer, size_t capacity,
				    size_t *head, size_t *tail)
{


	size_t free_space = (*tail <= *head) ? (capacity - *head + *tail - 1) :
					       (*tail - *head - 1);

	if (free_space == 0)
		return 0;

	size_t to_read = (free_space < capacity - *head) ? free_space :
							   (capacity - *head);

	ssize_t bytes_read = read(fd, buffer + *head, to_read);

	if (bytes_read > 0) {
		*head = (*head + bytes_read) % capacity;
		return bytes_read;
	}

	if (bytes_read == 0)
		return 0;

	return (bytes_read < 0 && errno == EAGAIN) ? 0 : (size_t)-1;
}

static inline size_t framed_buf_read(uint8_t *buffer, size_t capacity,
				     size_t *head, size_t *tail,
				     uint8_t *output, size_t output_capacity)
{
	if (*tail == *head)
		return 0;

	size_t available_data = (*head >= *tail) ? (*head - *tail) :
						   (capacity - *tail + *head);
	if (available_data < 2)
		return 0;

	uint16_t length = (buffer[*tail] << 8) | buffer[(*tail + 1) % capacity];
	if (available_data < 2 + length)
		return 0;

	if (length > output_capacity)
		return (size_t)-1;

	for (size_t i = 0; i < length; ++i) {
		output[i] = buffer[(*tail + 2 + i) % capacity];
	}

	*tail = (*tail + 2 + length) % capacity;
	return length;
}

static inline size_t framed_read_delim(uint8_t *buffer, size_t capacity,
				       size_t *head, size_t *tail,
				       uint8_t *output, size_t output_capacity)
{
	size_t output_pos = 0;
	while (*tail != *head) {
		size_t available_data = (*head >= *tail) ?
						(*head - *tail) :
						(capacity - *tail + *head);
		if (available_data < 2)
			break;

		uint16_t length = (buffer[*tail] << 8) |
				  buffer[(*tail + 1) % capacity];

		if (available_data < 2 + length)
			break;

		if (output_pos + length + 1 > output_capacity)
			break;

		for (size_t i = 0; i < length; ++i) {
			output[output_pos++] =
				buffer[(*tail + 2 + i) % capacity];
		}

		output[output_pos++] = '\n';
		*tail = (*tail + 2 + length) % capacity;
	}

	return output_pos;
}

static inline size_t feed_fd(int fd, unsigned char *buf, size_t size,
			     size_t *rpos, size_t *wpos)
{
	if (circ_space(*rpos, *wpos, size) == 0)
		return 0;
	size_t to_read = (*wpos >= *rpos) ? size - *wpos : *rpos - *wpos - 1;
	ssize_t n = read(fd, buf + *wpos, to_read);
	if (n <= 0)
		return (size_t)n;
	*wpos = (*wpos + (size_t)n) % size;
	return (size_t)n;
}

#endif
