#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>

#include "ttl/mmap.h"

using namespace ttl;

class MMAPTest : public ::testing::Test {
 protected:
	// You can define per-test set-up logic as usual.
	virtual void SetUp() {
		char buf[L_tmpnam];
		auto res = tmpnam(buf);
		ASSERT_NE(res, nullptr);
		mmap_file = res;
		std::ofstream file(mmap_file, std::ios::trunc | std::ios::binary);
		file.write(data.data(), static_cast<std::streamsize>(data.size()));
		file.close();
	}

	// You can define per-test tear-down logic as usual.
	virtual void TearDown() {
		if(!mmap_file.empty()) {
			ASSERT_EQ(remove(mmap_file.c_str()), 0);
			mmap_file.clear();
		}
	}

	// Some expensive resource shared by all tests.
	static std::string mmap_file;
	static std::string data;
};

std::string MMAPTest::mmap_file;
std::string MMAPTest::data = "Hello World, how are you ?";

TEST_F(MMAPTest, MapFile) {
	ttl::mmap map;

	ASSERT_EQ(nullptr, map.data());
	ASSERT_FALSE(map.is_valid());
	ASSERT_TRUE(map.open(mmap_file));
	ASSERT_TRUE(map.is_valid());
	ASSERT_NE(nullptr, map.data());
	ASSERT_EQ(map.data(), &map[0]);
	ASSERT_EQ(map.data(), &map.at(0));
	ASSERT_EQ(data.size(), map.size());
	ASSERT_TRUE(memcmp(data.data(), map.data(), map.size()) == 0);
}

TEST_F(MMAPTest, MapFileFailed) {
	ttl::mmap map;

	ASSERT_EQ(nullptr, map.data());
	ASSERT_FALSE(map.is_valid());
	ASSERT_FALSE(map.open("mmap_test_not_existent.txt"));
	ASSERT_FALSE(map.is_valid());
	ASSERT_EQ(nullptr, map.data());
}

TEST_F(MMAPTest, MapFileConstructor) {
	const std::string data = "Hello World, how are you ?";

	ASSERT_THROW([]() {
		ttl::mmap map("mmap_test_not_existent.txt");
	}(), std::runtime_error);

	ttl::mmap map(mmap_file);
	ASSERT_TRUE(map.is_valid());
	ASSERT_NE(nullptr, map.data());
	ASSERT_EQ(map.data(), &map[0]);
	ASSERT_EQ(map.data(), &map.at(0));
	ASSERT_EQ(data.size(), map.size());
	ASSERT_TRUE(memcmp(data.data(), map.data(), map.size()) == 0);
}
