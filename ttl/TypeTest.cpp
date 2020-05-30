#include <gtest/gtest.h>
#include "ttl/type.h"

using ttl::type;

TEST(TypeTest, Types) {
	auto type = type::create<int* const>();

#ifdef __linux__
	ASSERT_EQ("int* const", type.pretty_name());
	ASSERT_EQ("int", type.base_type().pretty_name());
#else
#ifdef _WIN64
	ASSERT_EQ("int * __ptr64 const", type.pretty_name());
#else
	ASSERT_EQ("int * const", type.pretty_name());
#endif
	ASSERT_EQ("int", type.base_type().pretty_name());
#endif

	ASSERT_FALSE(type.is_fundamental());
	ASSERT_TRUE(type.is_const());
	ASSERT_FALSE(type.is_volatile());
	ASSERT_FALSE(type.is_lvalue_ref());
	ASSERT_FALSE(type.is_rvalue_ref());
	ASSERT_TRUE(type.is_pointer());

	ASSERT_TRUE(type.base_type().is_fundamental());
	ASSERT_FALSE(type.base_type().is_const());
	ASSERT_FALSE(type.base_type().is_volatile());
	ASSERT_FALSE(type.base_type().is_lvalue_ref());
	ASSERT_FALSE(type.base_type().is_rvalue_ref());
	ASSERT_FALSE(type.base_type().is_pointer());
}

TEST(TypeTest, DefaultConstruct) {
	auto type = type::create<unsigned int>();

	ASSERT_EQ("unsigned int", type.pretty_name());
	ASSERT_TRUE(type.is_fundamental());
	
	auto ptr = type.create_object();
	ASSERT_FALSE(ptr == nullptr);
}
