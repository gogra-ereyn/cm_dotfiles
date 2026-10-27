#include "stats.h"

void mpv_window_add(mpv_window_t *w, double now, uint64_t bytes)
{
	mpv_bucket_t *b;
	int i;
	double cutoff;

	if (w->count == 0) {
		w->head = 0;
		w->count = 1;
		b = &w->buckets[0];
		b->t_end = now + w->bucket_sec;
		b->bytes = 0;
	}

	cutoff = now - w->span;
	for (i = 0; i < w->count; ++i) {
		int idx = (w->head + MPV_MAX_BUCKETS - i - 1) % MPV_MAX_BUCKETS;
		b = &w->buckets[idx];
		if (b->t_end >= cutoff)
			break;
		w->bytes_total -= b->bytes;
		b->bytes = 0;
		b->t_end = 0.0;
		--w->count;
	}

	b = &w->buckets[w->head];
	while (now > b->t_end) {
		int next = (w->head + 1) % MPV_MAX_BUCKETS;
		w->head = next;
		if (w->count < MPV_MAX_BUCKETS)
			++w->count;
		b = &w->buckets[w->head];
		w->bytes_total -= b->bytes;
		b->bytes = 0;
		b->t_end = w->buckets[(w->head + MPV_MAX_BUCKETS - 1) % MPV_MAX_BUCKETS].t_end +
			   w->bucket_sec;
	}

	b->bytes += bytes;
	w->bytes_total += bytes;
}

double mpv_window_rate(const mpv_window_t *w, double now)
{
	double oldest, effective;

	if (w->count == 0)
		return 0.0;

	oldest = w->buckets[(w->head + MPV_MAX_BUCKETS - w->count + 1) % MPV_MAX_BUCKETS].t_end -
		 w->bucket_sec;
	effective = now - oldest;
	if (effective <= 0.0)
		return 0.0;
	if (effective > w->win_sec)
		effective = w->win_sec;
	return (double)w->bytes_total / effective;
}
