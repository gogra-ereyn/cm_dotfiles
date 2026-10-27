#ifndef CUTEID_H
#define CUTEID_H

#include <stdint.h>

/* cuteid epoch: 2026-01-01T00:00:00Z as unix seconds */
#define CUTEID_EPOCH 1767225600ULL

/* current schema version embedded in v8 uuids */
#define CUTEID_SCHEMA 0

struct uuid {
	uint8_t b[16];
};

/*
 * entity fields packed into a uuidv8.
 *
 * 122-bit payload (MSB first):
 *
 *   [121:80]  start_time      42 bits   ms since cuteid epoch
 *   [79:76]   server_id        4 bits
 *   [75:72]   schema           4 bits
 *   [71:64]   (reserved)       8 bits
 *   [63:32]   process_handle  32 bits
 *   [31:0]    alert_handle    32 bits   0 = process entity
 *
 * mapped into uuid128 with version=8 at bits [79:76]
 * and variant=0b10 at bits [63:62]:
 *
 *   uuid[127:80] = payload[121:74]   48 bits
 *   uuid[79:76]  = 0x8               version
 *   uuid[75:64]  = payload[73:62]    12 bits
 *   uuid[63:62]  = 0b10              variant
 *   uuid[61:0]   = payload[61:0]     62 bits
 */
struct cuteid_fields {
	uint64_t start_time;
	uint32_t process_handle;
	uint32_t alert_handle;
	uint8_t  server_id;
	uint8_t  schema;
};

/* ---- hex helpers ---- */

static inline int hexv(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

/* ---- uuid parse / format ---- */

static inline int uuid_parse(struct uuid *u, const char *s)
{
	int bi = 0, h, l;

	while (*s == ' ' || *s == '\t') s++;
	while (*s && bi < 16) {
		if (*s == '-') { s++; continue; }
		h = hexv(*s++);
		l = *s ? hexv(*s++) : -1;
		if (h < 0 || l < 0) return -1;
		u->b[bi++] = (h << 4) | l;
	}
	while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t') s++;
	return (bi == 16 && *s == '\0') ? 0 : -1;
}

/* hyphens: 1 = 8-4-4-4-12, 0 = 32 hex chars */
static inline int uuid_format(char *out, const struct uuid *u, int hyphens)
{
	static const char hex[] = "0123456789abcdef";
	static const int grp[] = {4, 2, 2, 2, 6};
	char *p = out;
	int gi = 0, gc = 0;
	int i;

	for (i = 0; i < 16; i++) {
		*p++ = hex[u->b[i] >> 4];
		*p++ = hex[u->b[i] & 0xF];
		gc++;
		if (hyphens && gc == grp[gi] && gi < 4) {
			*p++ = '-';
			gc = 0;
			gi++;
		}
	}
	*p = '\0';
	return (int)(p - out);
}

/* ---- uuid inspection ---- */

static inline int uuid_version(const struct uuid *u)
{
	return (u->b[6] >> 4) & 0xF;
}

/* returns: 0=ncs, 1=rfc9562, 2=microsoft, 3=future */
static inline int uuid_variant(const struct uuid *u)
{
	uint8_t v = u->b[8];
	if ((v & 0x80) == 0)    return 0;
	if ((v & 0xC0) == 0x80) return 1;
	if ((v & 0xE0) == 0xC0) return 2;
	return 3;
}

/* ---- cuteid v8 encode / decode ---- */

static inline void cuteid_encode(struct uuid *u, const struct cuteid_fields *f)
{
	uint64_t ts  = f->start_time & 0x3FFFFFFFFFFULL;
	uint8_t  sid = f->server_id & 0xF;
	uint8_t  sch = f->schema & 0xF;
	uint32_t ph  = f->process_handle;
	uint32_t ah  = f->alert_handle;
	uint64_t a, b, c, hi, lo;
	int i;

	/* region a: payload[121:74] */
	a = (ts << 6) | ((uint64_t)sid << 2) | (sch >> 2);

	/* region b: payload[73:62] */
	b = ((uint64_t)(sch & 0x3) << 10) | ((uint64_t)(ph >> 30) & 0x3);

	/* region c: payload[61:0] */
	c = ((uint64_t)(ph & 0x3FFFFFFFU) << 32) | ah;

	hi = (a << 16) | (0x8ULL << 12) | b;
	lo = (0x2ULL << 62) | c;

	for (i = 0; i < 8; i++) u->b[i]     = (hi >> (56 - i * 8)) & 0xFF;
	for (i = 0; i < 8; i++) u->b[8 + i] = (lo >> (56 - i * 8)) & 0xFF;
}

static inline void cuteid_decode(struct cuteid_fields *f, const struct uuid *u)
{
	uint64_t hi = 0, lo = 0;
	uint64_t a, b, c;
	int i;

	for (i = 0; i < 8; i++) hi = (hi << 8) | u->b[i];
	for (i = 0; i < 8; i++) lo = (lo << 8) | u->b[8 + i];

	a = (hi >> 16) & 0xFFFFFFFFFFFFULL;
	b = hi & 0xFFF;
	c = lo & 0x3FFFFFFFFFFFFFFFULL;

	f->start_time     = (a >> 6) & 0x3FFFFFFFFFFULL;
	f->server_id      = (a >> 2) & 0xF;
	f->schema         = (uint8_t)(((a & 0x3) << 2) | ((b >> 10) & 0x3));
	f->process_handle = (uint32_t)(((b & 0x3) << 30) | (c >> 32));
	f->alert_handle   = (uint32_t)(c & 0xFFFFFFFF);
}

#endif /* CUTEID_H */
