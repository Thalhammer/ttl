#include <gtest/gtest.h>

#include "include/ttl/binary_reader.h"
#include "include/ttl/binary_writer.h"
#include <sstream>

using namespace ttl;

TEST(BinaryReaderWriterTest, WriteThenRead) {
	{
		std::stringstream sstream;
		binary_writer wrt(sstream);
		binary_reader rdr(sstream);

		wrt.write(int32_t(10));

		ASSERT_EQ(4, sstream.str().size());

		auto read = rdr.read_int32();
		ASSERT_EQ(10, read);
	}

	{
		std::stringstream sstream;
		binary_writer wrt(sstream);
		binary_reader rdr(sstream);

		std::string orig("Hello World");

		wrt.write(orig);
		auto str = sstream.str();

		ASSERT_EQ(12, str.size());
		ASSERT_EQ(11, str[0]);
		ASSERT_EQ(orig, str.substr(1));

		auto read = rdr.read_string();
		ASSERT_EQ(orig, read);
	}
}