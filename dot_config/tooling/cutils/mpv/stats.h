#ifndef MPV_STATS_H
#define MPV_STATS_H

#include <stdint.h>

#ifndef MPV_MAX_BUCKETS
#define MPV_MAX_BUCKETS 120
#endif

typedef struct mpv_stats {
	uint64_t bytes_total;
	uint64_t bytes_interval;

	double t_start;
	double t_now;
	double t_prev; // in hindsight, not sure how useful this is
	double t_first_byte;
	double t_last_byte;

	double rate_inst;
	double rate_peak_inst;
} mpv_stats_t;

typedef struct mpv_bucket {
	double t_end;
	uint64_t bytes;
} mpv_bucket_t;

typedef struct mpv_window {
	mpv_bucket_t buckets[MPV_MAX_BUCKETS];
	int head;
	int count;
	double span;
	uint64_t bytes_total;
	double win_sec;
	double bucket_sec;
} mpv_window_t;

void mpv_window_add(mpv_window_t *w, double now, uint64_t bytes);

double mpv_window_rate(const mpv_window_t *w, double now);

#endif
