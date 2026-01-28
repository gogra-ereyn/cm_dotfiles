#include <gtest/gtest.h>
#include "cbuf.h"

#define ASSERT_PEEK_STREQ(buf_expr, offset_expr, expected_expr)                                   \
	do {                                                                                      \
		auto &b_ = (buf_expr);                                                            \
		const size_t off_ = (offset_expr);                                                \
		const char *exp_ = (expected_expr);                                               \
		const size_t len_ = std::strlen(exp_);                                            \
		ASSERT_GE(b_.size(), off_ + len_)                                                 \
			<< "buffer too small for expected string at offset " << off_ << " (need " \
			<< (off_ + len_) << ", have " << b_.size() << ")";                        \
		for (size_t i_ = 0; i_ < len_; ++i_) {                                            \
			const char actual_ = static_cast<char>(b_.peek_at(off_ + i_));            \
			ASSERT_EQ(actual_, exp_[i_])                                              \
				<< "mismatch at offset " << off_ << " + " << i_ << " (abs "       \
				<< (off_ + i_) << ", phys "                                       \
				<< RingBuf<RingBufTest::CAP>::index(b_.cons_head + off_ + i_)     \
				<< "): expected '" << exp_[i_] << "', got '" << actual_ << "'";   \
		}                                                                                 \
	} while (0)

class RingBufTest : public ::testing::Test {
    protected:
	static constexpr size_t CAP = 16;
	RingBuf<CAP> buf;

	void fill(const char *str)
	{
		size_t len = std::strlen(str);
		ASSERT_GE(buf.space_contig(), len);
		std::memcpy(buf.write_ptr(), str, len);
		buf.produce(len);
	}

	void fill_at(size_t idx, const char *str)
	{
		size_t len = std::strlen(str);
		for (size_t i = 0; i < len; i++)
			buf.data()[(idx + i) & (CAP - 1)] = str[i];
	}
};

TEST_F(RingBufTest, InitialState)
{
	EXPECT_EQ(buf.size(), 0);
	EXPECT_EQ(buf.space(), CAP);
	EXPECT_TRUE(buf.empty());
	EXPECT_FALSE(buf.full());
}

TEST_F(RingBufTest, CountAndSpaceAfterProduce)
{
	buf.produce(5);
	EXPECT_EQ(buf.size(), 5);
	EXPECT_EQ(buf.space(), 11);
}

TEST_F(RingBufTest, CountAndSpaceAfterProduceAndConsume)
{
	buf.produce(10);
	buf.consume(3);
	EXPECT_EQ(buf.size(), 7);
	EXPECT_EQ(buf.space(), 9);
}

TEST_F(RingBufTest, FullState)
{
	buf.produce(CAP);
	EXPECT_TRUE(buf.full());
	EXPECT_FALSE(buf.empty());
	EXPECT_EQ(buf.size(), CAP);
	EXPECT_EQ(buf.space(), 0);
}

TEST_F(RingBufTest, CountHandlesHeadWrap)
{
	buf.prod_head = SIZE_MAX - 2;
	buf.cons_head = SIZE_MAX - 5;
	EXPECT_EQ(buf.size(), 3);

	buf.produce(4);
	EXPECT_EQ(buf.size(), 7);
}

TEST_F(RingBufTest, IndexMasks)
{
	EXPECT_EQ(RingBuf<CAP>::index(0), 0);
	EXPECT_EQ(RingBuf<CAP>::index(5), 5);
	EXPECT_EQ(RingBuf<CAP>::index(16), 0);
	EXPECT_EQ(RingBuf<CAP>::index(17), 1);
	EXPECT_EQ(RingBuf<CAP>::index(SIZE_MAX), 15);
}

TEST_F(RingBufTest, CountContigNoWrap)
{
	buf.produce(5);
	EXPECT_EQ(buf.size_contig(), 5);
	EXPECT_FALSE(buf.count_wraps());
}

TEST_F(RingBufTest, CountContigWithWrap)
{
	const size_t start = CAP - 4;
	buf.cons_head = start;
	buf.prod_head = start + 8;

	EXPECT_EQ(buf.size(), 8);
	EXPECT_EQ(buf.size_contig(), 4);
	EXPECT_TRUE(buf.count_wraps());
}

TEST_F(RingBufTest, CountContigEmpty)
{
	EXPECT_EQ(buf.size_contig(), 0);
}

TEST_F(RingBufTest, SpaceContigNoWrap)
{
	EXPECT_EQ(buf.space_contig(), CAP);
	EXPECT_FALSE(buf.space_wraps());
}

TEST_F(RingBufTest, SpaceContigWithWrap)
{
	const size_t cons = 4;
	const size_t prod = 12;
	buf.cons_head = cons;
	buf.prod_head = prod;

	EXPECT_EQ(buf.space(), 8);
	EXPECT_EQ(buf.space_contig(), 4);
	EXPECT_TRUE(buf.space_wraps());
}

TEST_F(RingBufTest, SpaceContigFull)
{
	buf.produce(CAP);
	EXPECT_EQ(buf.space_contig(), 0);
}

TEST_F(RingBufTest, PeekAt)
{
	fill("abc");
	ASSERT_PEEK_STREQ(buf, 0, "abc");
}

TEST_F(RingBufTest, PeekAtWithWrap)
{
	const size_t start = CAP - 2;
	buf.cons_head = start;
	buf.prod_head = start + 4;
	fill_at(start, "wxyz");

	ASSERT_PEEK_STREQ(buf, 0, "wxyz");
}

TEST_F(RingBufTest, PeekAtAfterConsume)
{
	fill("abc");
	buf.consume(1);

	ASSERT_PEEK_STREQ(buf, 0, "bc");
}

TEST_F(RingBufTest, PeekAtWithOffset)
{
	fill("hello");

	ASSERT_PEEK_STREQ(buf, 0, "hello");
	ASSERT_PEEK_STREQ(buf, 2, "llo");
}

TEST_F(RingBufTest, ReadPtr)
{
	fill("hello");
	EXPECT_EQ(buf.read_ptr(), buf.data());
	EXPECT_EQ(*buf.read_ptr(), 'h');

	buf.consume(2);
	EXPECT_EQ(buf.read_ptr(), buf.data() + 2);
	EXPECT_EQ(*buf.read_ptr(), 'l');
}

TEST_F(RingBufTest, ReadPtrWraps)
{
	buf.cons_head = CAP + 2;
	EXPECT_EQ(buf.read_ptr(), buf.data() + 2);
}

TEST_F(RingBufTest, WritePtr)
{
	EXPECT_EQ(buf.write_ptr(), buf.data());

	buf.produce(5);
	EXPECT_EQ(buf.write_ptr(), buf.data() + 5);
}

TEST_F(RingBufTest, WritePtrWraps)
{
	buf.prod_head = CAP + 4;
	EXPECT_EQ(buf.write_ptr(), buf.data() + 4);
}

TEST_F(RingBufTest, IsContiguous)
{
	buf.produce(5);
	EXPECT_TRUE(buf.is_contiguous(5));
	EXPECT_TRUE(buf.is_contiguous(3));
	EXPECT_TRUE(buf.is_contiguous(0));
}

TEST_F(RingBufTest, IsContiguousWithWrap)
{
	const size_t start = CAP - 2;
	buf.cons_head = start;
	buf.prod_head = start + 6;

	EXPECT_TRUE(buf.is_contiguous(2));
	EXPECT_FALSE(buf.is_contiguous(4));
}

TEST_F(RingBufTest, CopyOutContiguous)
{
	fill("hello");
	char dst[6] = {};
	size_t n = buf.copy_out(reinterpret_cast<uint8_t *>(dst), 5);

	EXPECT_EQ(n, 5);
	EXPECT_STREQ(dst, "hello");
	ASSERT_PEEK_STREQ(buf, 0, "hello");
}

TEST_F(RingBufTest, CopyOutWrapped)
{
	const size_t start = CAP - 2;
	buf.cons_head = start;
	buf.prod_head = start + 5;
	fill_at(start, "world");

	char dst[6] = {};
	size_t n = buf.copy_out(reinterpret_cast<uint8_t *>(dst), 5);

	EXPECT_EQ(n, 5);
	EXPECT_STREQ(dst, "world");
	ASSERT_PEEK_STREQ(buf, 0, "world");
}

TEST_F(RingBufTest, CopyOutPartial)
{
	fill("hello");
	char dst[3] = {};
	size_t n = buf.copy_out(reinterpret_cast<uint8_t *>(dst), 2);

	EXPECT_EQ(n, 2);
	EXPECT_EQ(dst[0], 'h');
	EXPECT_EQ(dst[1], 'e');
	ASSERT_PEEK_STREQ(buf, 0, "hello");
}

TEST_F(RingBufTest, CopyOutClampedToAvailable)
{
	fill("hi");
	char dst[10] = {};
	size_t n = buf.copy_out(reinterpret_cast<uint8_t *>(dst), 10);

	EXPECT_EQ(n, 2);
	EXPECT_STREQ(dst, "hi");
	ASSERT_PEEK_STREQ(buf, 0, "hi");
}

TEST_F(RingBufTest, CopyOutDoesNotConsume)
{
	fill("test");
	char dst[4];
	buf.copy_out(reinterpret_cast<uint8_t *>(dst), 4);

	EXPECT_EQ(buf.size(), 4);
	ASSERT_PEEK_STREQ(buf, 0, "test");
}

TEST_F(RingBufTest, CopyOutEmpty)
{
	char dst[4] = "xxx";
	size_t n = buf.copy_out(reinterpret_cast<uint8_t *>(dst), 4);

	EXPECT_EQ(n, 0);
	EXPECT_STREQ(dst, "xxx");
}

TEST_F(RingBufTest, Reset)
{
	buf.produce(10);
	buf.consume(3);
	buf.reset();

	EXPECT_EQ(buf.cons_head, 0);
	EXPECT_EQ(buf.prod_head, 0);
	EXPECT_TRUE(buf.empty());
	EXPECT_EQ(buf.space(), CAP);
}
