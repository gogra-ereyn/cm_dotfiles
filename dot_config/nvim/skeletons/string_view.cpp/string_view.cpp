#include <string>
#include <string_view>
#include <unordered_map>

struct sv_hash {
	using is_transparent = void;

	size_t operator()(std::string_view s) const noexcept
	{
		return std::hash<std::string_view>{}(s);
	}
	size_t operator()(const std::string &s) const noexcept
	{
		return std::hash<std::string_view>{}(std::string_view(s));
	}
	size_t operator()(const char *s) const noexcept
	{
		return std::hash<std::string_view>{}(std::string_view(s));
	}
};

struct sv_eq {
	using is_transparent = void;

	bool operator()(std::string_view a, std::string_view b) const noexcept
	{
		return a == b;
	}
	bool operator()(const std::string &a, std::string_view b) const noexcept
	{
		return std::string_view(a) == b;
	}
	bool operator()(std::string_view a, const std::string &b) const noexcept
	{
		return a == std::string_view(b);
	}
	bool operator()(const std::string &a, const std::string &b) const noexcept
	{
		return a == b;
	}
};

// clang-format off
template <class V>
using umap_str = std::unordered_map<std::string, V, sv_hash, sv_eq>;
// clang-format on

// TODO not workin' yet
void usage()
{
	umap_str<int> m;
	m.emplace("alpha", 1);

	auto it1 = m.find("alpha"); // const char* (no alloc)
	auto it2 = m.find(std::string_view("alpha")); // string_view (no alloc)

	std::string s = "alpha";
	auto it3 = m.find(s); // std::string
}
