#include <gtest/gtest.h>

#include "timer.h"

using thalhammer::timer;

TEST(TimerTest, TimedExecute) {
	bool executed = false;
	std::mutex mtx;
	std::condition_variable cv;

	timer t;
	auto token = t.schedule([&]() {
		std::unique_lock<std::mutex> lck(mtx);
		executed = true;
		cv.notify_all();
	}, std::chrono::milliseconds(10));

	{	// Wait at most half a second for task to get executed.
		std::unique_lock<std::mutex> lck(mtx);
		EXPECT_TRUE(cv.wait_until(lck, std::chrono::steady_clock::now() + std::chrono::milliseconds(500), [&executed]() { return executed; }));
	}
}

TEST(TimerTest, CatchException) {
	bool executed = false;
	std::mutex mtx;
	std::condition_variable cv;

	{
		timer t;
		t.set_exception_handler([&](auto ex) {
			EXPECT_TRUE(ex);
			std::unique_lock<std::mutex> lck(mtx);
			executed = true;
			cv.notify_all();
		});

		auto token = t.schedule([&]() {
			throw std::exception("HELP");
		}, std::chrono::milliseconds(10));

		{	// Wait at most half a second for task to get executed.
			std::unique_lock<std::mutex> lck(mtx);
			EXPECT_TRUE(cv.wait_until(lck, std::chrono::steady_clock::now() + std::chrono::milliseconds(500), [&executed]() { return executed; }));
		}
	}
}