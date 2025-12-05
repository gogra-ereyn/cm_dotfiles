struct entity_keys {
	uint64_t app_id;
	uint64_t alert_id;
};

struct FrameStore {
    std::vector<uint32_t> offsets;
    std::vector<uint32_t> lengths;
    std::vector<entity_keys> keys;
    std::atomic<uint32_t> head;
};

struct Snapshot {
    std::vector<uint32_t> indices;
    uint32_t end;
};

struct U64Hasher {
    std::size_t operator()(uint64_t x) const noexcept {
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return static_cast<std::size_t>(x);
    }
};

class LatestSnapshotBuilder {
public:
    explicit LatestSnapshotBuilder(FrameStore& store)
        : store_(store)

    Snapshot build(uint32_t end) {
        Snapshot snap;
        std::unordered_map<uint64_t, uint32_t, U64Hasher> latest_app;
        std::unordered_map<uint64_t, uint32_t, U64Hasher> latest_alert;
        uint32_t approx;
        uint32_t i;
        entity_keys ek;

        snap.end = end;
        snap.indices.clear();

        approx = end ? end / 8u : 0u;
        if (approx < 16u) {
            approx = 16u;
        }
        latest_app.reserve(approx);
        latest_alert.reserve(approx);

        i = end;
        while (i > 0u) {
            i -= 1u;
            ek = store_.keys[i];

            if (ek.app_id != 0u && ek.alert_id == 0u) {
                if (latest_app.find(ek.app_id) == latest_app.end()) {
                    latest_app.emplace(ek.app_id, i);
                }
            } else if (ek.alert_id != 0u) {
                if (latest_alert.find(ek.alert_id) == latest_alert.end()) {
                    latest_alert.emplace(ek.alert_id, i);
                }
            }
        }

        snap.indices.reserve(latest_app.size() + latest_alert.size());

        auto it_a = latest_app.begin();
        auto it_a_end = latest_app.end();
        for (; it_a != it_a_end; ++it_a) {
            snap.indices.push_back(it_a->second);
        }

        auto it_l = latest_alert.begin();
        auto it_l_end = latest_alert.end();
        for (; it_l != it_l_end; ++it_l) {
            snap.indices.push_back(it_l->second);
        }

        std::sort(snap.indices.begin(), snap.indices.end());
        return snap;
    }

private:
    FrameStore& store_;
};

