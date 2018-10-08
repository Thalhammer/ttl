#include <gtest/gtest.h>

#include "include/ttl/process.h"

using ttl::process;

TEST(ProcessTest, ExecuteProcess) {

	process p;
#ifdef _WIN32
	ASSERT_TRUE(p.open(R"(C:\Windows\System32\cmd.exe)", { "/C", "echo Hello World" }));
#else
	ASSERT_TRUE(p.open(R"(/bin/echo)", { "Hello World" }));
#endif
	//ASSERT_TRUE(p.is_alive());
	ASSERT_TRUE(p.errormsg().empty());
	auto & out = p.get_stdout();
	std::string line;
	std::getline(out, line);
#ifdef _WIN32
	ASSERT_EQ("Hello World\r", line);
#else
	ASSERT_EQ("Hello World", line);
	#endif
	while (std::getline(out, line)); // Read remaining output
	ASSERT_TRUE(p.wait());
	ASSERT_FALSE(p.is_alive());
	ASSERT_EQ(0, p.exitcode());
}
