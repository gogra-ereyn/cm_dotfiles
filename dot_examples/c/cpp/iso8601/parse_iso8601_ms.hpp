#pragma once

#include <cstdint>
#include <chrono>

namespace iso8601_ms
{

using tp_ms = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

static inline int dig(char c)
{
	unsigned v;
	v = (unsigned)(c - '0');
	return v <= 9u ? (int)v : -1;
}

static inline bool parse_2(const char *p, int &out)
{
	int a, b;
	a = dig(p[0]);
	b = dig(p[1]);
	if (a < 0 || b < 0)
		return false;
	out = a * 10 + b;
	return true;
}

static inline bool parse_4(const char *p, int &out)
{
	int a, b, c, d;
	a = dig(p[0]);
	b = dig(p[1]);
	c = dig(p[2]);
	d = dig(p[3]);
	if (a < 0 || b < 0 || c < 0 || d < 0)
		return false;
	out = a * 1000 + b * 100 + c * 10 + d;
	return true;
}

static inline bool is_leap(int y)
{
	return (y % 4 == 0) && ((y % 100 != 0) || (y % 400 == 0));
}

static inline int64_t days_from_civil(int y, unsigned m, unsigned d)
{
	int era;
	unsigned yoe, doy, doe;
	y -= m <= 2;
	era = (y >= 0 ? y : y - 399) / 400;
	yoe = (unsigned)(y - era * 400);
	doy = (153u * (m + (m > 2 ? -3 : 9)) + 2u) / 5u + d - 1u;
	doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
	return (int64_t)era * 146097 + (int64_t)doe - 719468;
}

static inline bool parse(const char *s, tp_ms &out)
{
	int y, mo, da, hh, mm, ss;
	const char *p;
	int frac_ms;
	int tz_sign, tzh, tzm;
	int mdays;
	int64_t days, sec, off;

	if (!s)
		return false;

	if (!parse_4(s + 0, y))
		return false;
	if (s[4] != '-')
		return false;
	if (!parse_2(s + 5, mo))
		return false;
	if (s[7] != '-')
		return false;
	if (!parse_2(s + 8, da))
		return false;
	if (s[10] != 'T' && s[10] != ' ')
		return false;
	if (!parse_2(s + 11, hh))
		return false;
	if (s[13] != ':')
		return false;
	if (!parse_2(s + 14, mm))
		return false;
	if (s[16] != ':')
		return false;
	if (!parse_2(s + 17, ss))
		return false;

	if (mo < 1 || mo > 12)
		return false;
	if (hh < 0 || hh > 23)
		return false;
	if (mm < 0 || mm > 59)
		return false;
	if (ss < 0 || ss > 60)
		return false;

	static const int mdays_norm[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	mdays = mdays_norm[mo - 1] + (mo == 2 && is_leap(y) ? 1 : 0);
	if (da < 1 || da > mdays)
		return false;

	p = s + 19;

	frac_ms = 0;
	if (*p == '.') {
		int digits, v;
		++p;
		digits = 0;

		v = dig(*p);
		if (v < 0)
			return false;

		while (digits < 3) {
			v = dig(*p);
			if (v < 0)
				break;
			frac_ms = frac_ms * 10 + v;
			++p;
			++digits;
		}
		while (digits < 3) {
			frac_ms *= 10;
			++digits;
		}
		while (dig(*p) >= 0)
			++p;
	}

	tz_sign = 0;
	tzh = 0;
	tzm = 0;

	if (*p == 'Z' || *p == 'z') {
		++p;
		tz_sign = 0;
	} else if (*p == '+' || *p == '-') {
		tz_sign = (*p == '-') ? -1 : +1;
		++p;
		if (!parse_2(p, tzh))
			return false;
		p += 2;
		if (*p == ':')
			++p;
		if (!parse_2(p, tzm))
			return false;
		p += 2;
		if (tzh < 0 || tzh > 23)
			return false;
		if (tzm < 0 || tzm > 59)
			return false;
	} else {
		return false;
	}

	if (*p != '\0')
		return false;

	days = days_from_civil(y, (unsigned)mo, (unsigned)da);
	sec = days * 86400 + (int64_t)hh * 3600 + (int64_t)mm * 60 + (int64_t)ss;

	if (tz_sign != 0) {
		off = (int64_t)tzh * 3600 + (int64_t)tzm * 60;
		sec -= (int64_t)tz_sign * off;
	}

	out = tp_ms(std::chrono::milliseconds(sec * 1000 + frac_ms));
	return true;
}

static inline bool parse_ms_since_epoch(const char *s, int64_t &out_ms)
{
	tp_ms tp;
	if (!parse(s, tp))
		return false;
	out_ms = tp.time_since_epoch().count();
	return true;
}

} // namespace iso8601_ms
