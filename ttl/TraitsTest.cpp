#include <gtest/gtest.h>
#include "include/ttl/traits.h"
#include <array>
#include <set>
#include <map>
#include <unordered_set>

using namespace ttl;

TEST(TraitsTest, IsIterable) {
	ASSERT_TRUE(traits::is_iterable<std::vector<int>>::value);
	ASSERT_TRUE((traits::is_iterable<std::array<int, 10>>::value));
	ASSERT_TRUE(traits::is_iterable<std::set<int>>::value);
	ASSERT_TRUE(traits::is_iterable<std::unordered_set<int>>::value);
	ASSERT_TRUE((traits::is_iterable<std::map<int,int>>::value));
	ASSERT_FALSE(traits::is_iterable<int>::value);
}
