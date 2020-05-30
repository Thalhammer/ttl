#include <gtest/gtest.h>

#include "ttl/string_util.h"
#include "ttl/cxx11_helpers.h"

using namespace ttl;

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

	auto str = string::join(ttl::cbegin(parts), ttl::cend(parts), " ");
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

TEST(StringTest, Replace) {
	const static std::string test_str = "Hello sunny world";
	{
		std::string in = test_str;
		string::replace(in, "sunny", "rainy");
		ASSERT_EQ("Hello rainy world", in);
	}
	{
		std::string in = test_str;
		string::replace(in, "sunny", "sunny sunny");
		ASSERT_EQ("Hello sunny sunny world", in);
	}
	{
		std::string in = test_str;
		auto out = string::replace_copy(in, "sunny", "rainy");
		ASSERT_EQ("Hello rainy world", out);
		ASSERT_EQ(test_str, in);
	}
	{
		std::string in = test_str;
		auto out = string::replace_copy(in, "sunny", "sunny sunny");
		ASSERT_EQ("Hello sunny sunny world", out);
		ASSERT_EQ(test_str, in);
	}
}

TEST(StringTest, Length) {
	ASSERT_EQ(11, string::length("Hello World"));
	ASSERT_EQ(11, string::length(std::string("Hello World")));
}
