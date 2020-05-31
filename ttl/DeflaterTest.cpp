#include <gtest/gtest.h>
#include <fstream>
#include <array>

#include "ttl/io/deflater.h"
#include "ttl/io/deflate_stream.h"

using ttl::io::deflater;
using ttl::io::deflate_ostream;
using ttl::io::deflate_istream;

const std::string test_in = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
const unsigned char test_out[] = {
	0x78, 0xda, 0xed, 0x90, 0xc1, 0x6d, 0x03, 0x31, 0x0c, 0x04, 0x5b, 0xd9, 0x02, 0x0e, 0xd7, 0x83, 0xff, 0xfe,
	0xa5, 0x02, 0x9e, 0x44, 0x1c, 0x18, 0x8b, 0xe2, 0x59, 0x22, 0x0f, 0x70, 0xf7, 0xa1, 0xe3, 0x3c, 0xfc, 0x49,
	0x2a, 0xc8, 0x97, 0xcb, 0x9d, 0x5d, 0xec, 0xd5, 0x06, 0x2b, 0xe4, 0x98, 0xa1, 0xa8, 0xd6, 0x6c, 0x60, 0x8a,
	0x83, 0x94, 0x7d, 0x41, 0xb1, 0x3e, 0xd9, 0xd9, 0x23, 0x8f, 0x54, 0xf3, 0xa7, 0x48, 0xdf, 0xc1, 0x4d, 0x7c,
	0x2c, 0x98, 0x5c, 0x51, 0x85, 0x14, 0xdd, 0x7a, 0xe8, 0x03, 0x2c, 0x43, 0xad, 0xc2, 0x59, 0x8f, 0x84, 0x48,
	0x3f, 0xa5, 0x46, 0x77, 0x84, 0xa3, 0xd1, 0x96, 0x21, 0x60, 0x7f, 0x05, 0x30, 0x94, 0xf6, 0x4e, 0xa0, 0x26,
	0xf7, 0x78, 0x24, 0x80, 0x07, 0xf9, 0x1b, 0xef, 0xb4, 0x16, 0x87, 0x07, 0xad, 0xb8, 0x38, 0x4e, 0x1e, 0x06,
	0xb6, 0xf9, 0x74, 0x53, 0x29, 0x31, 0x9f, 0xff, 0x8e, 0xcf, 0x98, 0x6e, 0xa8, 0x61, 0x3f, 0xc8, 0x6f, 0x9d,
	0x09, 0x83, 0xb7, 0xd0, 0x15, 0x1f, 0x59, 0x1a, 0x25, 0x7b, 0x12, 0x6e, 0x34, 0x2b, 0xf6, 0xd8, 0x78, 0xec,
	0x83, 0xfb, 0x92, 0x6d, 0x33, 0x89, 0xe0, 0x74, 0x13, 0xa5, 0xd4, 0x27, 0xf5, 0xe2, 0x91, 0xfe, 0xe9, 0xb8,
	0xfe, 0xba, 0xc5, 0xfa, 0x87, 0xf6, 0xbf, 0xd3, 0xdb, 0x4e, 0x5f, 0x6f, 0x87, 0xd7, 0xc9
};

TEST(DeflaterTest, Deflate) {
	deflater def;
	std::array<uint8_t, 4096> buf;

	def.set_input(reinterpret_cast<const uint8_t*>(test_in.data()), test_in.size());
	def.set_output(buf.data(), buf.size());
	size_t read = 0, written = 0;
	def.finish();
	ASSERT_TRUE(def.compress(read, written, false));
	ASSERT_TRUE(def.finished());
	ASSERT_TRUE(def.need_input());
	ASSERT_FALSE(def.need_output());

	ASSERT_EQ(written, sizeof(test_out));
	ASSERT_TRUE(memcmp(buf.data(), test_out, sizeof(test_out)) == 0);
}

TEST(DeflaterTest, DeflateSmallOutbuf) {
	deflater def;
	std::array<uint8_t, 64> buf;

	def.set_input(reinterpret_cast<const uint8_t*>(test_in.data()), test_in.size());
	def.set_output(buf.data(), buf.size());
	size_t read = 0, written = 0;
	while (!def.finished()) {
		if (!def.compress(read, written))
			FAIL();
		if (def.need_input()) def.finish();
		if (def.need_output()) def.set_output(buf.data(), buf.size());
	}
	ASSERT_TRUE(def.finished());
	ASSERT_TRUE(def.need_input());
}

TEST(DeflaterTest, DeflateStatic) {
	auto buf = deflater::compress(reinterpret_cast<const uint8_t*>(test_in.data()), test_in.size());
	ASSERT_EQ(sizeof(test_out), buf.size());
	ASSERT_TRUE(memcmp(buf.data(), test_out, sizeof(test_out)) == 0);
}

TEST(DeflaterTest, DeflateOStream) {
	std::ostringstream ss;
	deflate_ostream strm(ss);
	strm << test_in;
	strm.finish();

	auto buf = ss.str();
	ASSERT_EQ(sizeof(test_out), buf.size());
	ASSERT_TRUE(memcmp(buf.data(), test_out, sizeof(test_out)) == 0);
}

TEST(DeflaterTest, DeflateIStream) {
	std::istringstream ss(test_in);
	deflate_istream strm(ss);

	std::string res;
	while (strm) {
		std::string b(1024, '\0');
		b.resize(static_cast<size_t>(strm.read(const_cast<char*>(b.data()), static_cast<std::streamsize>(b.size())).gcount()));
		res += b;
	}
	ASSERT_EQ(sizeof(test_out), res.size());
	ASSERT_TRUE(memcmp(res.data(), test_out, sizeof(test_out)) == 0);
}
