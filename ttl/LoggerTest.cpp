#include <gtest/gtest.h>
#include <sstream>

#include "logger.h"
#include "string_util.h"

using thalhammer::logger;
using thalhammer::loglevel;
using thalhammer::logmodule;
namespace string = thalhammer::string;

TEST(LoggerTest, LogOutput) {
	std::ostringstream logout;
	logout << std::endl;
	const std::string endl = logout.str();
	logout.str("");

	logger log(logout);

	// Check default loglevel
	EXPECT_EQ(loglevel::INFO, log.get_loglevel());

	{
		log << loglevel::INFO << logmodule("test") << "Hello Logger";
		EXPECT_TRUE(string::ends_with(logout.str(), " | INFO  | test | Hello Logger" + endl));
	}
}

TEST(LoggerTest, LevelCheck) {
	std::ostringstream logout;
	logout << std::endl;
	const std::string endl = logout.str();
	logout.str("");

	logger log(logout);

	// Check default loglevel
	EXPECT_EQ(loglevel::INFO, log.get_loglevel());
	log.set_loglevel(loglevel::WARN);
	EXPECT_EQ(loglevel::WARN, log.get_loglevel());

	{
		log << loglevel::INFO << logmodule("test") << "Hello Logger";
		EXPECT_TRUE(logout.str().empty());
	}
}


TEST(LoggerTest, LogOutputShort) {
	std::ostringstream logout;
	logout << std::endl;
	const std::string endl = logout.str();
	logout.str("");

	logger log(logout);

	// Check default loglevel
	EXPECT_EQ(loglevel::INFO, log.get_loglevel());

	{
		log(loglevel::INFO,"test") << std::string("Hello Logger");
		EXPECT_TRUE(string::ends_with(logout.str(), " | INFO  | test | Hello Logger" + endl));
	}
}
