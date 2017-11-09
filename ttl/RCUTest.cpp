#include <gtest/gtest.h>

#include <ttl/rcu.h>

using namespace thalhammer;

TEST(RCUTest, RCUValueConstruct) {
    rcu<std::string> test("Test");
	auto ptr = test.get();
	ASSERT_NE(ptr, nullptr);
	ASSERT_EQ("Test", *ptr);
}

TEST(RCUTest, RCUUpdateCopy) {
    rcu<std::string> test("Test");
	auto old = test.get();
	ASSERT_NE(old, nullptr);
	ASSERT_EQ("Test", *old);

	bool called = false;
	test.update([&called, old](std::string& str){
		ASSERT_NE(old.get(), &str);
		ASSERT_EQ("Test", str);
		called = true;
	});
	ASSERT_TRUE(called);

	auto ptr = test.get();
	ASSERT_NE(old, ptr);
}

TEST(RCUTest, RCUUpdateThrow) {
    rcu<std::string> test;
	auto old = test.get();

	bool called = false;
	ASSERT_THROW(test.update([&called](std::string& str){
		called = true;
		throw std::runtime_error("");
	}), std::runtime_error);
	ASSERT_TRUE(called);

	auto ptr = test.get();
	ASSERT_EQ(old, ptr);
}