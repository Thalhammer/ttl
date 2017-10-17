#include <gtest/gtest.h>

#include <ttl/signal.h>


TEST(SignalTest, Executed) {
	bool executed = false;
	
	thalhammer::signal<> sig;
	auto token = sig.add([&executed]() {
		executed = true;
	});
	ASSERT_FALSE(!token);
	sig();
	ASSERT_TRUE(executed);
}

TEST(SignalTest, OperatorPlus) {
	bool executed = false;

	thalhammer::signal<> sig;
	auto token = sig += [&executed]() {
		executed = true;
	};
	ASSERT_FALSE(!token);
	sig();
	ASSERT_TRUE(executed);
}

TEST(SignalTest, Params) {
	bool executed = false;

	thalhammer::signal<const std::string&> sig;
	auto token = sig.add([&executed](auto& str) {
		executed = true;
		ASSERT_EQ("Hello", str);
	});
	ASSERT_FALSE(!token);
	sig("Hello");
	ASSERT_TRUE(executed);
}

TEST(SignalTest, TokenCleared) {
	bool executed = false;

	thalhammer::signal<> sig;
	auto token = sig.add([&executed]() {
		executed = true;
	});
	ASSERT_FALSE(!token);
	token.reset();
	ASSERT_TRUE(!token);
	sig();
	ASSERT_FALSE(executed);
}
