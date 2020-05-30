#include <gtest/gtest.h>

#include "ttl/predicate.h"
#if __cplusplus >= 201402L
#include "ttl/linq.h"
using ttl::linq;
#endif

using namespace ttl::predicate;

TEST(PredicateTest, BasicPredicates) {
	auto peq = equals(10);
	auto pne = not_equals(10);
	auto plt = less(10);
	auto ple = less_equals(10);
	auto pgt = greater(10);
	auto pge = greater_equals(10);

	ASSERT_TRUE(peq(10));
	ASSERT_FALSE(peq(11));
	
	ASSERT_TRUE(pne(11));
	ASSERT_FALSE(pne(10));

	ASSERT_TRUE(plt(9));
	ASSERT_FALSE(plt(10));
	ASSERT_FALSE(plt(11));

	ASSERT_TRUE(ple(9));
	ASSERT_TRUE(ple(10));
	ASSERT_FALSE(ple(11));

	ASSERT_FALSE(pgt(9));
	ASSERT_FALSE(pgt(10));
	ASSERT_TRUE(pgt(11));

	ASSERT_FALSE(pge(9));
	ASSERT_TRUE(pge(10));
	ASSERT_TRUE(pge(11));
}

TEST(PredicateTest, Compound) {
	auto por = either (eq(10), eq(12));
	auto pand = both (gt(10), lt(12));
	auto pxor = oneof (ge(10), [](int x) { return x % 2 == 1; });

	ASSERT_FALSE(por(9));
	ASSERT_TRUE(por(10));
	ASSERT_FALSE(por(11));
	ASSERT_TRUE(por(12));
	ASSERT_FALSE(por(13));

	ASSERT_FALSE(pand(9));
	ASSERT_FALSE(pand(10));
	ASSERT_TRUE(pand(11));
	ASSERT_FALSE(pand(12));
	ASSERT_FALSE(pand(13));

	ASSERT_FALSE(pxor(8)); // 0 0
	ASSERT_TRUE(pxor(9)); // 0 1
	ASSERT_TRUE(pxor(10)); // 1 0
	ASSERT_FALSE(pxor(11)); // 1 1
}

#if __cplusplus >= 201402L
TEST(PredicateTest, LINQPredicate) {
	int data[] = { 10, 20, 30, 40 };

	auto res = linq(data)
		.where(ne(20))
		.to_vector();
	ASSERT_EQ(3, res.size());
	ASSERT_EQ(10, res[0]);
	ASSERT_EQ(30, res[1]);
	ASSERT_EQ(40, res[2]);
}
#endif
