#include <gtest/gtest.h>

#include "string_util.h"

using namespace thalhammer;

TEST(StringTest, Trim) {
	const static std::string test_str = "   test  ";
	{
		std::string in = test_str;
		string::ltrim(in);
		EXPECT_EQ("test  ", in);
	}
	{
		std::string in = test_str;
		string::rtrim(in);
		EXPECT_EQ("   test", in);
	}
	{
		std::string in = test_str;
		string::trim(in);
		EXPECT_EQ("test", in);
	}
	{
		std::string in = test_str;
		auto out = string::ltrim_copy(in);
		EXPECT_EQ("test  ", out);
		EXPECT_EQ(test_str, in);
	}
	{
		std::string in = test_str;
		auto out = string::rtrim_copy(in);
		EXPECT_EQ("   test", out);
		EXPECT_EQ(test_str, in);
	}
	{
		std::string in = test_str;
		auto out = string::trim_copy(in);
		EXPECT_EQ("test", out);
		EXPECT_EQ(test_str, in);
	}
}

TEST(StringTest, Split) {
	const static std::string test_str = "Hello World, how are you ?";
	{
		auto parts = string::split(test_str, std::string(" "));
		EXPECT_EQ(6, parts.size());
		EXPECT_EQ("Hello", parts[0]);
		EXPECT_EQ("World,", parts[1]);
		EXPECT_EQ("how", parts[2]);
		EXPECT_EQ("are", parts[3]);
		EXPECT_EQ("you", parts[4]);
		EXPECT_EQ("?", parts[5]);
	}
	{
		auto parts = string::split(test_str, std::string(" "), 3);
		EXPECT_EQ(3, parts.size());
		EXPECT_EQ("Hello", parts[0]);
		EXPECT_EQ("World,", parts[1]);
		EXPECT_EQ("how are you ?", parts[2]);
	}
}

TEST(StringTest, Join) {
	const static std::vector<std::string> parts = { "Hello", "World,", "how", "are", "you", "?" };

	auto str = string::join(std::cbegin(parts), std::cend(parts), " ");
	EXPECT_EQ("Hello World, how are you ?", str);
}

TEST(StringTest, StartEnd) {
	const static std::string test_str = "Hello World";

	EXPECT_TRUE(string::starts_with(test_str, std::string("Hello")));
	EXPECT_FALSE(string::starts_with(test_str, std::string("Het")));
	EXPECT_TRUE(string::ends_with(test_str, std::string("World")));
	EXPECT_FALSE(string::ends_with(test_str, std::string("t")));
}