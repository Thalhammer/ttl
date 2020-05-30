#include <gtest/gtest.h>

#include "ttl/crc.h"

using namespace ttl;

TEST(CRCTest, CalcCRC) {
	/* Its templated so we only check one of each bitsize */
	std::string data = "Sampledata";

	ASSERT_EQ(0xD2, CRC_8::get_crc(data));
	ASSERT_EQ(0x7E0, CRC_12::get_crc(data));
	ASSERT_EQ(0x3110, CRC_15::get_crc(data));
	ASSERT_EQ(0xC80D, CRC_16::get_crc(data));
	ASSERT_EQ(0x319269, CRC_24::get_crc(data));
	ASSERT_EQ(0xFF64BEF8, CRC_32::get_crc(data));
	ASSERT_EQ(0xBA1444EC9D210150, CRC_64::get_crc(data));
}
