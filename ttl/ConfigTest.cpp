#include <gtest/gtest.h>

#include "ttl/config.h"

using ttl::config;

TEST(ConfigTest, ReadConfig) {
	std::istringstream data(R"(
normal=Value
spaces=Value with spaces
empty=

#This is a comment
	tabs=get removed
values=    get trimmed    
)");

	config cfg;
	ASSERT_TRUE(cfg.read(data));
	ASSERT_TRUE(cfg.errormsg().empty());

	std::string value;
	ASSERT_TRUE(cfg.get("normal", value));
	ASSERT_EQ("Value", value);
	ASSERT_TRUE(cfg.get("spaces", value));
	ASSERT_EQ("Value with spaces", value);
	ASSERT_TRUE(cfg.get("empty", value));
	ASSERT_EQ("", value);
	ASSERT_TRUE(cfg.get("tabs", value));
	ASSERT_EQ("get removed", value);
	ASSERT_TRUE(cfg.get("values", value));
	ASSERT_EQ("get trimmed", value);
}

TEST(ConfigTest, Iterator) {
	std::istringstream data(R"(
normal=Value
spaces=Value with spaces
empty=

#This is a comment
	tabs=get removed
values=    get trimmed    
)");

	config cfg;
	ASSERT_TRUE(cfg.read(data));
	ASSERT_TRUE(cfg.errormsg().empty());

	ASSERT_EQ(5, cfg.size());
	// Just check compile
	for (auto& entry : cfg) {}
}

TEST(ConfigTest, ManualSet) {
	std::istringstream data(R"(
normal=Value
)");

	config cfg;
	ASSERT_TRUE(cfg.read(data));
	ASSERT_TRUE(cfg.errormsg().empty());

	std::string value;
	ASSERT_TRUE(cfg.get("normal", value));
	ASSERT_EQ("Value", value);

	cfg.set("normal", "Other Value");
	ASSERT_TRUE(cfg.get("normal", value));
	ASSERT_EQ("Other Value", value);

	ASSERT_FALSE(cfg.get("new", value));

	cfg.set("new", "Added Value");
	ASSERT_TRUE(cfg.get("new", value));
	ASSERT_EQ("Added Value", value);
}

TEST(ConfigTest, Transaction) {
	config cfg;
	cfg.set("base", "Value");
	
	auto transaction = cfg.begin_transaction();

	std::string value;
	ASSERT_TRUE(transaction.get("base", value));
	ASSERT_EQ("Value", value);

	ASSERT_FALSE(transaction.changed());

	transaction.set("base", "Other Value");
	ASSERT_TRUE(transaction.get("base", value));
	ASSERT_EQ("Other Value", value);
	ASSERT_TRUE(cfg.get("base", value));
	ASSERT_EQ("Value", value);

	ASSERT_TRUE(transaction.changed());

	transaction.set("new", "Test");
	ASSERT_TRUE(transaction.get("new", value));
	ASSERT_EQ("Test", value);
	ASSERT_FALSE(cfg.get("new", value));

	transaction.commit();

	ASSERT_FALSE(transaction.changed());
	
	ASSERT_TRUE(cfg.get("new", value));
	ASSERT_EQ("Test", value);
	ASSERT_TRUE(cfg.get("base", value));
	ASSERT_EQ("Other Value", value);
}

TEST(ConfigTest, IncludeHandler) {
	std::istringstream data(R"(
normal=Value
include file.incl
)");

	config cfg;
	bool visited = false;
	cfg.set_include_handler([&visited](const std::string& file, config::transaction& trans) {
		EXPECT_EQ("file.incl", file);
		EXPECT_FALSE(visited);
		trans.set("example", "Include");
		visited = true;
		return true;
	});

	cfg.read(data);
	ASSERT_TRUE(visited);
	std::string value;
	ASSERT_TRUE(cfg.get("normal", value));
	ASSERT_EQ("Value", value);
	ASSERT_TRUE(cfg.get("example", value));
	ASSERT_EQ("Include", value);
}