#ifndef _BUF_H
#define _BUF_H
#pragma once

#include <stdlib.h>
#include <stdint.h>

typedef struct {
	uint16_t len;
	uint8_t data[];
} frame_t;

static inline frame_t *frame_init_checked(uint8_t *buf, size_t bufcap)
{
	if (!buf || bufcap < sizeof(frame_t))
		return 0;
	frame_t *frame = (frame_t *)buf;
	return frame;
}

static inline frame_t *frame_init(uint8_t *buf)
{
	return (frame_t *)buf;
}

static inline uint16_t frame_len(frame_t *frame)
{
	return (frame->len >> 8) | (frame->len << 8);
}

#endif
