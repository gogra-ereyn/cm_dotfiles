#include <unordered_map>
#include <algorithm>
#include <atomic>
#include <vector>

struct Snapshot {
	std::vector<uint32_t> indices;
	uint32_t end;
};

static inline uint32_t mix32(uint32_t x)
{
	x += 0x9e3779b9u;
	x = (x ^ (x >> 16)) * 0x85ebca6bu;
	x = (x ^ (x >> 13)) * 0xc2b2ae35u;
	x ^= x >> 16;
	return x;
}

struct U32Hasher {
	std::size_t operator()(uint32_t x) const noexcept
	{
		return static_cast<std::size_t>(mix32(x));
	}
};

class LatestSnapshotBuilder
{
public:
	explicit LatestSnapshotBuilder(FrameStore & store)
		: store_(store)
	{
	}

	Snapshot build(uint32_t end)
	{
		Snapshot snap;
		std::unordered_map<uint32_t, uint32_t, U32Hasher> latest;
		uint32_t approx;
		uint32_t i;
		uint32_t key;

		snap.end = end;
		snap.indices.clear();

		approx = end ? end / 8u : 0u;
		if (approx < 16u) {
			approx = 16u;
		}
		latest.reserve(approx);

		i = end;
		while (i > 0u) {
			i -= 1u;
			key = store_.entity_keys[i];
			if (latest.find(key) == latest.end()) {
				latest.emplace(key, i);
			}
		}

		snap.indices.reserve(latest.size());

		auto it = latest.begin();
		auto it_end = latest.end();
		for (; it != it_end; ++it) {
			snap.indices.push_back(it->second);
		}

		std::sort(snap.indices.begin(), snap.indices.end());
		return snap;
	}

private:
	FrameStore & store_;
};

static inline uint32_t make_app_key(uint32_t app_id)
{
	return app_id;
}

static inline uint32_t make_alert_key(uint32_t alert_id)
{
	return alert_id | 0x80000000u;
}

static inline bool is_alert_key(uint32_t key)
{
	return (key & 0x80000000u) != 0u;
}

static inline uint32_t key_id(uint32_t key)
{
	return key & 0x7fffffffu;
}

struct FrameStore {
	std::vector<uint32_t> offsets;
	std::vector<uint32_t> lengths;
	std::vector<uint32_t> entity_keys;
	std::vector<uint32_t> parent_ids;
	std::atomic<uint32_t> head;

	FrameStore()
		: head(0)
	{
	}

	uint32_t size() const
	{
		return head.load(std::memory_order_acquire);
	}

	void reserve(uint32_t n)
	{
		offsets.reserve(n);
		lengths.reserve(n);
		entity_keys.reserve(n);
		parent_ids.reserve(n);
	}

	uint32_t append_app(uint32_t offset, uint32_t length, uint32_t app_id)
	{
		uint32_t idx;
		idx = head.load(std::memory_order_relaxed);
		if (idx >= offsets.size()) {
			offsets.push_back(offset);
			lengths.push_back(length);
			entity_keys.push_back(make_app_key(app_id));
			parent_ids.push_back(0u);
		} else {
			offsets[idx] = offset;
			lengths[idx] = length;
			entity_keys[idx] = make_app_key(app_id);
			parent_ids[idx] = 0u;
		}
		head.store(idx + 1u, std::memory_order_release);
		return idx;
	}

	uint32_t append_alert(uint32_t offset, uint32_t length, uint32_t app_id, uint32_t alert_id)
	{
		uint32_t idx;
		idx = head.load(std::memory_order_relaxed);
		if (idx >= offsets.size()) {
			offsets.push_back(offset);
			lengths.push_back(length);
			entity_keys.push_back(make_alert_key(alert_id));
			parent_ids.push_back(app_id);
		} else {
			offsets[idx] = offset;
			lengths[idx] = length;
			entity_keys[idx] = make_alert_key(alert_id);
			parent_ids[idx] = app_id;
		}
		head.store(idx + 1u, std::memory_order_release);
		return idx;
	}
};
