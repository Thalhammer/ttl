#include <gtest/gtest.h>
#include <ttl/any.h>

using thalhammer::any;

TEST(AnyTest, AnyInt) {
	int i = 1;
	int& iref = i;

	any copy(i);
	any ref = any::create<int&>(iref);
	any ptr(&i);

	ASSERT_EQ(1, copy.get<int>());
	auto cloned = copy.clone();
	int* p = copy.get_pointer<int>();
	*p = 10;
	ASSERT_EQ(10, copy.get<int>());
	ASSERT_EQ(1, cloned.get<int>());
}
