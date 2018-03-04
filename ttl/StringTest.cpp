#include <gtest/gtest.h>

#include "include/ttl/string_util.h"

using namespace thalhammer;

TEST(StringTest, Trim) {
	const static std::string test_str = "   test  ";
	{
		std::string in = test_str;
		string::ltrim(in);
		ASSERT_EQ("test  ", in);
	}
	{
		std::string in = test_str;
		string::rtrim(in);
		ASSERT_EQ("   test", in);
	}
	{
		std::string in = test_str;
		string::trim(in);
		ASSERT_EQ("test", in);
	}
	{
		std::string in = test_str;
		auto out = string::ltrim_copy(in);
		ASSERT_EQ("test  ", out);
		ASSERT_EQ(test_str, in);
	}
	{
		std::string in = test_str;
		auto out = string::rtrim_copy(in);
		ASSERT_EQ("   test", out);
		ASSERT_EQ(test_str, in);
	}
	{
		std::string in = test_str;
		auto out = string::trim_copy(in);
		ASSERT_EQ("test", out);
		ASSERT_EQ(test_str, in);
	}
}

TEST(StringTest, Split) {
	const static std::string test_str = "Hello World, how are you ?";
	{
		auto parts = string::split(test_str, std::string(" "));
		ASSERT_EQ(6, parts.size());
		ASSERT_EQ("Hello", parts[0]);
		ASSERT_EQ("World,", parts[1]);
		ASSERT_EQ("how", parts[2]);
		ASSERT_EQ("are", parts[3]);
		ASSERT_EQ("you", parts[4]);
		ASSERT_EQ("?", parts[5]);
	}
	{
		auto parts = string::split(test_str, std::string(" "), 3);
		ASSERT_EQ(3, parts.size());
		ASSERT_EQ("Hello", parts[0]);
		ASSERT_EQ("World,", parts[1]);
		ASSERT_EQ("how are you ?", parts[2]);
	}
}

TEST(StringTest, Join) {
	const static std::vector<std::string> parts = { "Hello", "World,", "how", "are", "you", "?" };

	auto str = string::join(std::cbegin(parts), std::cend(parts), " ");
	ASSERT_EQ("Hello World, how are you ?", str);
}

TEST(StringTest, StartEnd) {
	const static std::string test_str = "Hello World";

	ASSERT_TRUE(string::starts_with(test_str, std::string("Hello")));
	ASSERT_FALSE(string::starts_with(test_str, std::string("Het")));
	ASSERT_TRUE(string::ends_with(test_str, std::string("World")));
	ASSERT_FALSE(string::ends_with(test_str, std::string("t")));
}

TEST(StringTest, CaseConv) {
	const static std::string test_str = "TestString";
	{
		std::string in = test_str;
		string::to_lower(in);
		ASSERT_EQ("teststring", in);
	}
	{
		std::string in = test_str;
		string::to_upper(in);
		ASSERT_EQ("TESTSTRING", in);
	}
	{
		std::string in = test_str;
		auto out = string::to_lower_copy(in);
		ASSERT_EQ("teststring", out);
		ASSERT_EQ(test_str, in);
	}
	{
		std::string in = test_str;
		auto out = string::to_upper_copy(in);
		ASSERT_EQ("TESTSTRING", out);
		ASSERT_EQ(test_str, in);
	}
}