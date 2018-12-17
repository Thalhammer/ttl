#include <gtest/gtest.h>
#include "include/ttl/optional.h"

using ttl::optional;

TEST(OptionalTest, Empty) {
	optional<std::string> e{};
    ASSERT_FALSE(e);
    ASSERT_FALSE(e.has_value());
}

TEST(OptionalTest, Value) {
	optional<std::string> e{"Hello"};
    ASSERT_TRUE(e);
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e.value(), "Hello");

    static_assert(std::is_same<decltype(e)::value_type, std::string>::value, "type missmatch");
}