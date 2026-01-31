#include <cstdint>
#include <cstring>
#include <string>

static int64_t parse_timestamp(const char *s)
{
	static constexpr int month_days[12] = { 0,   31,  59,  90,  120, 151,
						181, 212, 243, 273, 304, 334 };

	if (std::strlen(s) != 29)
		return -1;

	int year = (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0');
	int month = (s[5] - '0') * 10 + (s[6] - '0');
	int day = (s[8] - '0') * 10 + (s[9] - '0');
	int hour = (s[11] - '0') * 10 + (s[12] - '0');
	int min = (s[14] - '0') * 10 + (s[15] - '0');
	int sec = (s[17] - '0') * 10 + (s[18] - '0');
	int ms = (s[20] - '0') * 100 + (s[21] - '0') * 10 + (s[22] - '0');
	int tz_h = (s[25] - '0') * 10 + (s[26] - '0');
	int tz_m = (s[27] - '0') * 10 + (s[28] - '0');

	if (s[4] != '-' || s[7] != '-' || s[10] != 'T' || s[13] != ':' || s[16] != ':' ||
	    s[19] != '.' || s[23] != ':' || (s[24] != '+' && s[24] != '-')) {
		return -1;
	}

	if (month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || min > 59 || sec > 59 ||
	    ms > 999) {
		return -1;
	}

	int y = year - 1970;
	int leap_years = (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400 - 477;
	int64_t days = y * 365 + leap_years + month_days[month - 1] + (day - 1);
	bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	if (is_leap && month > 2) {
		days += 1;
	}

	int64_t epoch_ms = days * 86400000LL + hour * 3600000LL + min * 60000LL + sec * 1000LL + ms;
	int64_t tz_offset_ms = tz_h * 3600000LL + tz_m * 60000LL;
	if (s[24] == '+') {
		epoch_ms -= tz_offset_ms;
	} else {
		epoch_ms += tz_offset_ms;
	}

	return epoch_ms;
}

static inline int64_t days_from_civil(int y, unsigned m, unsigned d)
{
	y -= m <= 2;
	const int era = (y >= 0 ? y : y - 399) / 400;
	const unsigned yoe = static_cast<unsigned>(y - era * 400);
	const unsigned doy = (153 * (m + (m > 2 ? -3u : 9u)) + 2) / 5 + d - 1;
	const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(doe) - 719468;
}

static int64_t parse_timestamp_v2(std::string_view s)
{
	if (s.size() != 29)
		return -1;

	if (s[4] != '-' || s[7] != '-' || s[10] != 'T' || s[13] != ':' || s[16] != ':' ||
	    s[19] != '.' || s[23] != ':' || (s[24] != '+' && s[24] != '-')) {
		return -1;
	}

	auto is_d = [](char c) -> bool {
		unsigned v = static_cast<unsigned>(static_cast<unsigned char>(c)) -
			     static_cast<unsigned>('0');
		return v <= 9u;
	};

	const int digit_pos[] = { 0,  1,  2,  3,  5,  6,  8,  9,  11, 12, 14,
				  15, 17, 18, 20, 21, 22, 25, 26, 27, 28 };
	for (unsigned i = 0; i < sizeof(digit_pos) / sizeof(digit_pos[0]); ++i) {
		if (!is_d(s[static_cast<size_t>(digit_pos[i])]))
			return -1;
	}

	auto p2 = [&](size_t i) -> unsigned {
		return (static_cast<unsigned>(s[i] - '0') * 10u) +
		       static_cast<unsigned>(s[i + 1] - '0');
	};
	auto p3 = [&](size_t i) -> unsigned {
		return (static_cast<unsigned>(s[i] - '0') * 100u) +
		       (static_cast<unsigned>(s[i + 1] - '0') * 10u) +
		       static_cast<unsigned>(s[i + 2] - '0');
	};
	auto p4 = [&](size_t i) -> int {
		return static_cast<int>((static_cast<unsigned>(s[i] - '0') * 1000u) +
					(static_cast<unsigned>(s[i + 1] - '0') * 100u) +
					(static_cast<unsigned>(s[i + 2] - '0') * 10u) +
					static_cast<unsigned>(s[i + 3] - '0'));
	};

	int year;
	unsigned month, day, hour, min, sec, ms, tz_h, tz_m;
	year = p4(0);
	month = p2(5);
	day = p2(8);
	hour = p2(11);
	min = p2(14);
	sec = p2(17);
	ms = p3(20);
	tz_h = p2(25);
	tz_m = p2(27);

	if (month < 1 || month > 12)
		return -1;
	if (hour > 23 || min > 59 || sec > 59 || ms > 999)
		return -1;
	if (tz_m > 59)
		return -1;

	const bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	static constexpr unsigned dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned mdays = dim[month - 1] + ((month == 2 && is_leap) ? 1u : 0u);
	if (day < 1 || day > mdays)
		return -1;

	const int64_t days = days_from_civil(year, month, day);
	const int64_t tod_ms = static_cast<int64_t>(hour) * 3600000 +
			       static_cast<int64_t>(min) * 60000 +
			       static_cast<int64_t>(sec) * 1000 + static_cast<int64_t>(ms);

	int64_t epoch_ms = days * 86400000 + tod_ms;

	const int64_t tz_offset_ms =
		static_cast<int64_t>(tz_h) * 3600000 + static_cast<int64_t>(tz_m) * 60000;

	if (s[24] == '+')
		epoch_ms -= tz_offset_ms;
	else
		epoch_ms += tz_offset_ms;

	return epoch_ms;
}
