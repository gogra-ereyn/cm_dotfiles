#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <array>
#include <unordered_map>
#include <cstdlib>

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

	void to_bytes(uint8_t *out) const
	{
		memcpy(out, &hi, 8);
		memcpy(out + 8, &lo, 8);
	}

	struct uuid_hash {
		size_t operator()(const uuid_t &uuid) const noexcept
		{
			return static_cast<size_t>(uuid.lo);
		}
	};
};

static constexpr auto make_hex_lut()
{
	std::array<uint8_t, 256> t{};
	for (auto &v : t)
		v = 0x80;
	for (int i = 0; i <= 9; i++)
		t['0' + i] = i;
	for (int i = 0; i < 6; i++) {
		t['a' + i] = 10 + i;
		t['A' + i] = 10 + i;
	}
	return t;
}
static constexpr auto hex_val = make_hex_lut();

struct uuid_parse_result {
	uuid_t uuid;
	bool ok;
};

uuid_parse_result uuid_from_string(const char *str, size_t len)
{
	uuid_parse_result result = {};
	uint8_t bytes[16];
	size_t byte_idx = 0;
	size_t i = 0;

	while (i < len && byte_idx < 16) {
		if (str[i] == '-') {
			i++;
			continue;
		}

		if (i + 1 >= len) {
			return result;
		}

		uint8_t hi = hex_val[static_cast<uint8_t>(str[i])];
		uint8_t lo = hex_val[static_cast<uint8_t>(str[i + 1])];

		// being cute - validate both nibbles at same time
		if ((hi | lo) & 0x80)
			return result;

		bytes[byte_idx++] = (hi << 4) | lo;
		i += 2;
	}

	if (byte_idx != 16) {
		return result;
	}

	result.uuid = uuid_t::from_bytes(bytes);
	result.ok = true;
	return result;
}

uuid_parse_result uuid_from_string(const char *str)
{
	return uuid_from_string(str, strlen(str));
}

struct uuid_hash {
	size_t operator()(const uuid_t &uuid) const noexcept
	{
		return static_cast<size_t>(uuid.lo);
	}
};

namespace tests
{

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name)                        \
	do {                              \
		tests_run++;              \
		printf("  %-50s ", name); \
		fflush(stdout);           \
	} while (0)

#define PASS()                      \
	do {                        \
		tests_passed++;     \
		printf("[PASS]\n"); \
	} while (0)

#define FAIL(msg)                           \
	do {                                \
		printf("[FAIL] %s\n", msg); \
	} while (0)

#define ASSERT_EQ(a, b, msg)       \
	do {                       \
		if ((a) != (b)) {  \
			FAIL(msg); \
			return;    \
		}                  \
	} while (0)

#define ASSERT_TRUE(cond, msg)     \
	do {                       \
		if (!(cond)) {     \
			FAIL(msg); \
			return;    \
		}                  \
	} while (0)

void test_roundtrip_bytes_to_string_to_bytes()
{
	TEST("roundtrip: bytes -> uuid -> string -> uuid -> bytes");

	uint8_t original[16] = { 0x55, 0x0e, 0x84, 0x00, 0xe2, 0x9b, 0x41, 0xd4,
				 0xa7, 0x16, 0x44, 0x66, 0x55, 0x44, 0x00, 0x00 };

	uuid_t uuid1 = uuid_t::from_bytes(original);

	char str[37] = { 0 };
	uuid1.write_string(str);

	auto [uuid2, ok] = uuid_from_string(str);
	ASSERT_TRUE(ok, "parse failed");

	uint8_t final[16];
	uuid2.to_bytes(final);

	ASSERT_TRUE(memcmp(original, final, 16) == 0, "bytes mismatch");
	ASSERT_TRUE(uuid1 == uuid2, "uuid mismatch");

	PASS();
}

void test_roundtrip_string_to_uuid_to_string()
{
	TEST("roundtrip: string -> uuid -> string");

	const char *original = "550e8400-e29b-41d4-a716-446655440000";

	auto [uuid, ok] = uuid_from_string(original);
	ASSERT_TRUE(ok, "parse failed");

	char final[37] = { 0 };
	uuid.write_string(final);

	ASSERT_TRUE(strcmp(original, final) == 0, "string mismatch");
	PASS();
}

void test_parse_no_hyphens()
{
	TEST("parse: UUID without hyphens");

	auto [uuid, ok] = uuid_from_string("0123456789abcdef0123456789abcdef");
	ASSERT_TRUE(ok, "parse failed");

	char buf[37] = { 0 };
	uuid.write_string(buf);
	ASSERT_TRUE(strcmp(buf, "01234567-89ab-cdef-0123-456789abcdef") == 0, "mismatch");

	PASS();
}

void test_parse_uppercase()
{
	TEST("parse: uppercase -> lowercase output");

	auto [uuid, ok] = uuid_from_string("ABCDEF00-1234-5678-9ABC-DEF012345678");
	ASSERT_TRUE(ok, "parse failed");

	char buf[37] = { 0 };
	uuid.write_string(buf);
	ASSERT_TRUE(strcmp(buf, "abcdef00-1234-5678-9abc-def012345678") == 0, "mismatch");

	PASS();
}

void test_parse_rejects_invalid()
{
	TEST("parse: rejects invalid input");

	auto r1 = uuid_from_string("0123456g-89ab-cdef-0123-456789abcdef");
	ASSERT_TRUE(!r1.ok, "should reject 'g'");

	auto r2 = uuid_from_string("01234567-89ab-cdef");
	ASSERT_TRUE(!r2.ok, "should reject short input");

	auto r3 = uuid_from_string("");
	ASSERT_TRUE(!r3.ok, "should reject empty");

	PASS();
}

}

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

// EXAMPLE specialising std hash
namespace std
{
template <> struct hash<uuid_t> {
	size_t operator()(const uuid_t &uuid) const noexcept
	{
		return static_cast<size_t>(uuid.lo);
	}
};

};

int main()
{
	dprintf(2, "\n=== UUID Tests ===\n\n");

	dprintf(2, "roundtrip:\n");
	tests::test_roundtrip_bytes_to_string_to_bytes();
	tests::test_roundtrip_string_to_uuid_to_string();

	dprintf(2, "parsing:\n");
	tests::test_parse_no_hyphens();
	tests::test_parse_uppercase();
	tests::test_parse_rejects_invalid();

	dprintf(2, "\n======================\n");
	dprintf(2, "results: %d/%d passed\n", tests::tests_passed, tests::tests_run);
	dprintf(2, "======================\n\n");
	return (tests::tests_passed == tests::tests_run) ? 0 : 1;
}

//namespace table_based
//{
//struct uuid_t {
//	uint64_t hi;
//	uint64_t lo;
//
//	size_t write_string(char *buf) const noexcept
//	{
//		// lookup table: byte value -> two hex chars
//		// "00", "01", "02", ... "fe", "ff"
//		// clang-format off
//        static const char hex_lut[512] = {
//            '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7',
//            '0','8','0','9','0','a','0','b','0','c','0','d','0','e','0','f',
//            '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7',
//            '1','8','1','9','1','a','1','b','1','c','1','d','1','e','1','f',
//            '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7',
//            '2','8','2','9','2','a','2','b','2','c','2','d','2','e','2','f',
//            '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7',
//            '3','8','3','9','3','a','3','b','3','c','3','d','3','e','3','f',
//            '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7',
//            '4','8','4','9','4','a','4','b','4','c','4','d','4','e','4','f',
//            '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7',
//            '5','8','5','9','5','a','5','b','5','c','5','d','5','e','5','f',
//            '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7',
//            '6','8','6','9','6','a','6','b','6','c','6','d','6','e','6','f',
//            '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7',
//            '7','8','7','9','7','a','7','b','7','c','7','d','7','e','7','f',
//            '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7',
//            '8','8','8','9','8','a','8','b','8','c','8','d','8','e','8','f',
//            '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7',
//            '9','8','9','9','9','a','9','b','9','c','9','d','9','e','9','f',
//            'a','0','a','1','a','2','a','3','a','4','a','5','a','6','a','7',
//            'a','8','a','9','a','a','a','b','a','c','a','d','a','e','a','f',
//            'b','0','b','1','b','2','b','3','b','4','b','5','b','6','b','7',
//            'b','8','b','9','b','a','b','b','b','c','b','d','b','e','b','f',
//            'c','0','c','1','c','2','c','3','c','4','c','5','c','6','c','7',
//            'c','8','c','9','c','a','c','b','c','c','c','d','c','e','c','f',
//            'd','0','d','1','d','2','d','3','d','4','d','5','d','6','d','7',
//            'd','8','d','9','d','a','d','b','d','c','d','d','d','e','d','f',
//            'e','0','e','1','e','2','e','3','e','4','e','5','e','6','e','7',
//            'e','8','e','9','e','a','e','b','e','c','e','d','e','e','e','f',
//            'f','0','f','1','f','2','f','3','f','4','f','5','f','6','f','7',
//            'f','8','f','9','f','a','f','b','f','c','f','d','f','e','f','f',
//        };
//
//        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(this);
//        char* p = buf;
//        // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
//        // bytes:  0-3   4-5   6-7   8-9   10-15
//        #define WRITE_BYTE(i) do { \
//            const char* h = &hex_lut[bytes[i] * 2]; \
//            *p++ = h[0]; \
//            *p++ = h[1]; \
//        } while(0)
//
//        WRITE_BYTE(0); WRITE_BYTE(1); WRITE_BYTE(2); WRITE_BYTE(3);
//        *p++ = '-';
//        WRITE_BYTE(4); WRITE_BYTE(5);
//        *p++ = '-';
//        WRITE_BYTE(6); WRITE_BYTE(7);
//        *p++ = '-';
//        WRITE_BYTE(8); WRITE_BYTE(9);
//        *p++ = '-';
//        WRITE_BYTE(10); WRITE_BYTE(11); WRITE_BYTE(12);
//        WRITE_BYTE(13); WRITE_BYTE(14); WRITE_BYTE(15);
//
//        #undef WRITE_BYTE
//		// clang-format on
//		return 36;
//	}
//};
//
//// u128
//namespace bignum
//{
//
//using uuid_t = __uint128_t;
//
//struct uuid_hash {
//	size_t operator()(uuid_t uuid) const noexcept
//	{
//		uint64_t lo = static_cast<uint64_t>(uuid);
//		uint64_t hi = static_cast<uint64_t>(uuid >> 64);
//		return lo ^ (hi + 0x9e3779b97f4a7c15ull + (lo << 6) + (lo >> 2));
//	}
//};
//uuid_t uuid_from_bytes(const uint8_t *bytes)
//{
//	uuid_t result;
//	memcpy(&result, bytes, 16);
//	return result;
//}
//};
//
//namespace hashing
//{
///*
//    return a ^ (b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2));
//```
//
//this is a variant of the **boost::hash_combine** pattern. the magic constant `0x9e3779b97f4a7c15` is derived from the **golden ratio**:
//```
//φ = (1 + √5) / 2 ≈ 1.618...
//2^64 / φ ≈ 0x9e3779b97f4a7c15
//```
//
//this constant has good bit-mixing properties, iss essentially a way to spread bits around so that similar inputs produce very different outputs. the shifts (`<< 6`, `>> 2`) add additional mixing.
//
//**sources:**
//- boost's `hash_combine`: https://www.boost.org/doc/libs/release/libs/container_hash/doc/html/hash.html
//- the golden ratio constant is discussed in knuth's *the art of computer programming*,
//
//for a 128-bit uuid, you could also just return one half (since uuids should already be well-distributed), but combining both halves is safer if your uuids have any structure.
//
//## uuid string format
//the standard uuid string format is defined in rfc 4122 (https://datatracker.ietf.org/doc/html/rfc4122):
//xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
//   8    - 4  - 4  - 4  -    12
//   */
//}
//
//}
