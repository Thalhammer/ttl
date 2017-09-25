#include <gtest/gtest.h>

#include "config.h"

using thalhammer::config;

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
	EXPECT_TRUE(cfg.read(data));
	EXPECT_TRUE(cfg.errormsg().empty());

	std::string value;
	EXPECT_TRUE(cfg.get("normal", value));
	EXPECT_EQ("Value", value);
	EXPECT_TRUE(cfg.get("spaces", value));
	EXPECT_EQ("Value with spaces", value);
	EXPECT_TRUE(cfg.get("empty", value));
	EXPECT_EQ("", value);
	EXPECT_TRUE(cfg.get("tabs", value));
	EXPECT_EQ("get removed", value);
	EXPECT_TRUE(cfg.get("values", value));
	EXPECT_EQ("get trimmed", value);
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
	EXPECT_TRUE(cfg.read(data));
	EXPECT_TRUE(cfg.errormsg().empty());

	EXPECT_EQ(5, cfg.size());
	// Just check compile
	for (auto& entry : cfg) {}
}

TEST(ConfigTest, ManualSet) {
	std::istringstream data(R"(
normal=Value
)");

	config cfg;
	EXPECT_TRUE(cfg.read(data));
	EXPECT_TRUE(cfg.errormsg().empty());

	std::string value;
	EXPECT_TRUE(cfg.get("normal", value));
	EXPECT_EQ("Value", value);

	cfg.set("normal", "Other Value");
	EXPECT_TRUE(cfg.get("normal", value));
	EXPECT_EQ("Other Value", value);

	EXPECT_FALSE(cfg.get("new", value));

	cfg.set("new", "Added Value");
	EXPECT_TRUE(cfg.get("new", value));
	EXPECT_EQ("Added Value", value);
}

TEST(ConfigTest, Transaction) {
	config cfg;
	cfg.set("base", "Value");
	
	auto transaction = cfg.begin_transaction();

	std::string value;
	EXPECT_TRUE(transaction.get("base", value));
	EXPECT_EQ("Value", value);

	EXPECT_FALSE(transaction.changed());

	transaction.set("base", "Other Value");
	EXPECT_TRUE(transaction.get("base", value));
	EXPECT_EQ("Other Value", value);
	EXPECT_TRUE(cfg.get("base", value));
	EXPECT_EQ("Value", value);

	EXPECT_TRUE(transaction.changed());

	transaction.set("new", "Test");
	EXPECT_TRUE(transaction.get("new", value));
	EXPECT_EQ("Test", value);
	EXPECT_FALSE(cfg.get("new", value));

	transaction.commit();

	EXPECT_FALSE(transaction.changed());
	
	EXPECT_TRUE(cfg.get("new", value));
	EXPECT_EQ("Test", value);
	EXPECT_TRUE(cfg.get("base", value));
	EXPECT_EQ("Other Value", value);
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
	EXPECT_TRUE(visited);
	std::string value;
	EXPECT_TRUE(cfg.get("normal", value));
	EXPECT_EQ("Value", value);
	EXPECT_TRUE(cfg.get("example", value));
	EXPECT_EQ("Include", value);
}