#include <string>
#include <string_view>
#include <unordered_map>

struct sv_hash {
	using is_transparent = void;

	std::size_t operator()(std::string_view v) const noexcept
	{
		return std::hash<std::string_view>{}(v);
	}
	std::size_t operator()(const std::string &s) const noexcept
	{
		return (*this)(std::string_view{ s });
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
		return std::string_view{ a } == b;
	}
	bool operator()(std::string_view a, const std::string &b) const noexcept
	{
		return a == std::string_view{ b };
	}
};

template <class V> using sv_umap = std::unordered_map<std::string, V, sv_hash, sv_eq>;

int main()
{
	sv_umap<int> m;
	m.emplace("alpha", 1);

	std::string_view k = "alpha";
	if (auto it = m.find(k); it != m.end()) {
		(void)it->second;
	}

	bool has = m.contains(std::string_view{ "alpha" });
	(void)has;
}
