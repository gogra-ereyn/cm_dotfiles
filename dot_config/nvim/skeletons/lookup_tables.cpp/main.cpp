#include <array>

enum MsgType { MSG_INIT = 0, MSG_DATA = 5, MSG_CLOSE = 255, MSG_COUNT = 256 };

constexpr auto make_label_table()
{
	std::array<const char *, MSG_COUNT> t{};
	t[MSG_INIT] = "init";
	t[MSG_DATA] = "data";
	t[MSG_CLOSE] = "close";
	return t;
}

constexpr auto make_int_table()
{
	std::array<int, MSG_COUNT> t{};
    /* fill/set with a different default value*/
	for (auto &v : t)
		v = -1;

	t[MSG_INIT] = 12;
	t[MSG_DATA] = 6;
    t[MSG_CLOSE] = 4;
	return t;
}

constexpr auto label_table = make_label_table();
