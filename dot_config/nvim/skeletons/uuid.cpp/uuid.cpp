#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unordered_map>

namespace struct_based
{
struct uuid_t {
	uint64_t hi;
	uint64_t lo;
	// returns number of bytes written (always 36, excludes null)
	// caller ensures buf has at least 36 bytes available
	size_t write_string(char *buf) const noexcept
	{
		static const char hex[] = "0123456789abcdef";
		const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this);
		size_t out = 0;
		for (size_t i = 0; i < 16; ++i) {
			if (i == 4 || i == 6 || i == 8 || i == 10) {
				buf[out++] = '-';
			}
			buf[out++] = hex[bytes[i] >> 4];
			buf[out++] = hex[bytes[i] & 0x0f];
		}
		return out;
	}

	bool operator==(const uuid_t &other) const noexcept
	{
		return hi == other.hi && lo == other.lo;
	}

	bool operator<(const uuid_t &other) const noexcept
	{
		return hi < other.hi || (hi == other.hi && lo < other.lo);
	}

	static uuid_t from_bytes(const uint8_t *bytes)
	{
		uuid_t result;
		memcpy(&result.hi, bytes, 8);
		memcpy(&result.lo, bytes + 8, 8);
		return result;
	}

	struct uuid_hash {
		size_t operator()(const uuid_t &uuid) const noexcept
		{
			return static_cast<size_t>(uuid.lo);
		}
	};

	void usage()
	{
		char payload[1024];
		size_t offset = 0;
		uuid_t uuid{ 127334, 4455340 };
		offset += sprintf(payload, "{\"id\":\"");
		offset += uuid.write_string(payload + offset);
		offset += sprintf(payload + offset, "\",\"type\":%d}", 4);
		// or for unordered map
		// std::unordered_map<uuid_t, uint64_t, uuid_hash> mymap;
	}
};

}

// EXAMPLE specialising std hash
namespace std
{
template <> struct hash<struct_based::uuid_t> {
	size_t operator()(const struct_based::uuid_t &uuid) const noexcept
	{
		return static_cast<size_t>(uuid.lo);
	}
};
struct uuid_hash {
	size_t operator()(const struct_based::uuid_t &uuid) const noexcept
	{
		return static_cast<size_t>(uuid.lo);
	}
};

};

namespace table_based
{
struct uuid_t {
	uint64_t hi;
	uint64_t lo;

	size_t write_string(char *buf) const noexcept
	{
		// lookup table: byte value -> two hex chars
		// "00", "01", "02", ... "fe", "ff"
		// clang-format off
        static const char hex_lut[512] = {
            '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7',
            '0','8','0','9','0','a','0','b','0','c','0','d','0','e','0','f',
            '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7',
            '1','8','1','9','1','a','1','b','1','c','1','d','1','e','1','f',
            '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7',
            '2','8','2','9','2','a','2','b','2','c','2','d','2','e','2','f',
            '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7',
            '3','8','3','9','3','a','3','b','3','c','3','d','3','e','3','f',
            '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7',
            '4','8','4','9','4','a','4','b','4','c','4','d','4','e','4','f',
            '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7',
            '5','8','5','9','5','a','5','b','5','c','5','d','5','e','5','f',
            '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7',
            '6','8','6','9','6','a','6','b','6','c','6','d','6','e','6','f',
            '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7',
            '7','8','7','9','7','a','7','b','7','c','7','d','7','e','7','f',
            '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7',
            '8','8','8','9','8','a','8','b','8','c','8','d','8','e','8','f',
            '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7',
            '9','8','9','9','9','a','9','b','9','c','9','d','9','e','9','f',
            'a','0','a','1','a','2','a','3','a','4','a','5','a','6','a','7',
            'a','8','a','9','a','a','a','b','a','c','a','d','a','e','a','f',
            'b','0','b','1','b','2','b','3','b','4','b','5','b','6','b','7',
            'b','8','b','9','b','a','b','b','b','c','b','d','b','e','b','f',
            'c','0','c','1','c','2','c','3','c','4','c','5','c','6','c','7',
            'c','8','c','9','c','a','c','b','c','c','c','d','c','e','c','f',
            'd','0','d','1','d','2','d','3','d','4','d','5','d','6','d','7',
            'd','8','d','9','d','a','d','b','d','c','d','d','d','e','d','f',
            'e','0','e','1','e','2','e','3','e','4','e','5','e','6','e','7',
            'e','8','e','9','e','a','e','b','e','c','e','d','e','e','e','f',
            'f','0','f','1','f','2','f','3','f','4','f','5','f','6','f','7',
            'f','8','f','9','f','a','f','b','f','c','f','d','f','e','f','f',
        };

        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(this);
        char* p = buf;
        // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
        // bytes:  0-3   4-5   6-7   8-9   10-15
        #define WRITE_BYTE(i) do { \
            const char* h = &hex_lut[bytes[i] * 2]; \
            *p++ = h[0]; \
            *p++ = h[1]; \
        } while(0)

        WRITE_BYTE(0); WRITE_BYTE(1); WRITE_BYTE(2); WRITE_BYTE(3);
        *p++ = '-';
        WRITE_BYTE(4); WRITE_BYTE(5);
        *p++ = '-';
        WRITE_BYTE(6); WRITE_BYTE(7);
        *p++ = '-';
        WRITE_BYTE(8); WRITE_BYTE(9);
        *p++ = '-';
        WRITE_BYTE(10); WRITE_BYTE(11); WRITE_BYTE(12);
        WRITE_BYTE(13); WRITE_BYTE(14); WRITE_BYTE(15);

        #undef WRITE_BYTE
		// clang-format on
		return 36;
	}
};

// u128
namespace bignum
{

using uuid_t = __uint128_t;

struct uuid_hash {
	size_t operator()(uuid_t uuid) const noexcept
	{
		uint64_t lo = static_cast<uint64_t>(uuid);
		uint64_t hi = static_cast<uint64_t>(uuid >> 64);
		return lo ^ (hi + 0x9e3779b97f4a7c15ull + (lo << 6) + (lo >> 2));
	}
};
uuid_t uuid_from_bytes(const uint8_t *bytes)
{
	uuid_t result;
	memcpy(&result, bytes, 16);
	return result;
}
};

namespace hashing
{
/*
    return a ^ (b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2));
```

this is a variant of the **boost::hash_combine** pattern. the magic constant `0x9e3779b97f4a7c15` is derived from the **golden ratio**:
```
φ = (1 + √5) / 2 ≈ 1.618...
2^64 / φ ≈ 0x9e3779b97f4a7c15
```

this constant has good bit-mixing properties, iss essentially a way to spread bits around so that similar inputs produce very different outputs. the shifts (`<< 6`, `>> 2`) add additional mixing.

**sources:**
- boost's `hash_combine`: https://www.boost.org/doc/libs/release/libs/container_hash/doc/html/hash.html
- the golden ratio constant is discussed in knuth's *the art of computer programming*,

for a 128-bit uuid, you could also just return one half (since uuids should already be well-distributed), but combining both halves is safer if your uuids have any structure.

## uuid string format
the standard uuid string format is defined in rfc 4122 (https://datatracker.ietf.org/doc/html/rfc4122):
xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
   8    - 4  - 4  - 4  -    12
   */
}

}
