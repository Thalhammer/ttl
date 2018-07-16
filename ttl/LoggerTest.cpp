#include <gtest/gtest.h>
#include <sstream>

#include "include/ttl/logger.h"
#include "include/ttl/string_util.h"

using thalhammer::logger;
using thalhammer::streamlogger;
using thalhammer::loglevel;
using thalhammer::logmodule;
namespace string = thalhammer::string;

TEST(LoggerTest, LogOutput) {
	std::ostringstream logout;
	logout << std::endl;
	const std::string endl = logout.str();
	logout.str("");

	streamlogger log(logout);

	// Check default loglevel
	ASSERT_EQ(loglevel::INFO, log.get_loglevel());

	{
		log << loglevel::INFO << logmodule("test") << "Hello Logger";
		ASSERT_TRUE(string::ends_with(logout.str(), " | INFO  | test | Hello Logger" + endl));
	}
}

TEST(LoggerTest, LevelCheck) {
	std::ostringstream logout;

	streamlogger log(logout);

	// Check default loglevel
	ASSERT_EQ(loglevel::INFO, log.get_loglevel());
	log.set_loglevel(loglevel::WARN);
	ASSERT_EQ(loglevel::WARN, log.get_loglevel());

	{
		log << loglevel::INFO << logmodule("test") << "Hello Logger";
		ASSERT_TRUE(logout.str().empty());
	}
}

TEST(LoggerTest, LevelCheckInit) {
	std::ostringstream logout;

	streamlogger log(logout, loglevel::WARN);

	// Check default loglevel
	ASSERT_EQ(loglevel::WARN, log.get_loglevel());

	{
		log << loglevel::INFO << logmodule("test") << "Hello Logger";
		ASSERT_TRUE(logout.str().empty());
	}
}

TEST(LoggerTest, CheckFunction) {
	std::ostringstream logout;

	streamlogger log(logout);
	logger::check_function_t fn = [](loglevel l, const std::string& module, const std::string& message) {
		return module != "test";
	};
	// Check default loglevel
	ASSERT_EQ(loglevel::INFO, log.get_loglevel());
	ASSERT_FALSE(!(!log.get_check_function()));
	log.set_check_function(fn);
	ASSERT_TRUE(!(!log.get_check_function()));

	{
		log << loglevel::INFO << logmodule("test") << "Hello Logger";
		ASSERT_TRUE(logout.str().empty());
		log << loglevel::INFO << logmodule("test2") << "Hello Logger";
		ASSERT_FALSE(logout.str().empty());
	}
}

TEST(LoggerTest, LogOutputShort) {
	std::ostringstream logout;
	logout << std::endl;
	const std::string endl = logout.str();
	logout.str("");

	streamlogger log(logout);

	// Check default loglevel
	ASSERT_EQ(loglevel::INFO, log.get_loglevel());

	{
		log(loglevel::INFO,"test") << std::string("Hello Logger");
		ASSERT_TRUE(string::ends_with(logout.str(), " | INFO  | test | Hello Logger" + endl));
	}
}
