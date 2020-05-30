#include <gtest/gtest.h>
#include "ttl/any.h"

using ttl::any;

namespace AnyTest {
struct A {
	A() = default;
	A(const A&) = default;
	A(A&&) = default;
	virtual ~A() {}
	virtual void test() {
		std::cout<<"A"<<std::endl;
	}
	int x = 0;
};
struct B {
	B() = default;
	B(const B&) = default;
	B(B&&) = default;
	virtual ~B() {}
	int y;
	virtual void test2() {

	}
};
struct C : A, B {
	C() = default;
	C(const C&) = default;
	C(C&&) = default;
	virtual void test() {
		std::cout<<"C"<<std::endl;
	}
	int k = 1;
};
struct D : C {
	D() = default;
	D(const D&) = default;
	D(D&&) = default;
	virtual void test2() {

	}
};

struct E {
	E() = default;
	E(const E&) = default;
	E(E&&) = default;
	virtual ~E() {}
	virtual void x() {}
};
struct F : E {
};

struct G {
};
struct H : G {

};
}

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

TEST(AnyTest, UpCast) {
	using namespace AnyTest;
	any test(D{});
	ASSERT_EQ(typeid(D), test.std_type());

	D* ptr = test.get_pointer<D>();
	ASSERT_EQ(dynamic_cast<A*>(ptr), test.upcast<A>());
	ASSERT_EQ(dynamic_cast<B*>(ptr), test.upcast<B>());
	ASSERT_EQ(dynamic_cast<C*>(ptr), test.upcast<C>());
	ASSERT_EQ(dynamic_cast<D*>(ptr), test.upcast<D>());

	any test2(F{});
	ASSERT_EQ(typeid(F), test2.std_type());
	F* ptr2 = test2.get_pointer<F>();
	ASSERT_EQ(dynamic_cast<E*>(ptr2), test2.upcast<E>());
	ASSERT_EQ(dynamic_cast<F*>(ptr2), test2.upcast<F>());

#ifdef _WIN32
	// Windows needs a vtable to emit rtti data so check if we throw a exception
	any test3(H{});
	ASSERT_EQ(typeid(H), test3.std_type());
	H* ptr3 = test2.get_pointer<H>();
	ASSERT_THROW(test3.upcast<G>(), std::logic_error);
	ASSERT_THROW(test3.upcast<H>(), std::logic_error);
#else
	// Linux seems to support rtti for classes without a vtable, so test this also
	any test3(H{});
	ASSERT_EQ(typeid(H), test3.std_type());
	H* ptr3 = test3.get_pointer<H>();
	ASSERT_EQ(dynamic_cast<G*>(ptr3), test3.upcast<G>());
	ASSERT_EQ(dynamic_cast<H*>(ptr3), test3.upcast<H>());
#endif
	ASSERT_NE(nullptr, ptr3);
}

TEST(AnyTest, UpCastNotClass) {
	any test(10);
	ASSERT_THROW(test.upcast<std::string>(), std::logic_error);
}

struct strTest {
	std::string to_string() const {
		return "Test";
	}
};

TEST(AnyTest, ToString) {
	using namespace AnyTest;
	any test(10);
	ASSERT_EQ("10", test.to_string());
	any test2(A{});
	ASSERT_THROW(test2.to_string(), std::logic_error);
	any test3(strTest{});
	ASSERT_EQ("Test", test3.to_string());
}

TEST(AnyTest, Iterate) {
	std::vector<int> vec = { 10, 20, 30, 40 };
	{
		any test = ttl::any::create<std::vector<int>&>(vec);
		auto it = test.iterate();
		ASSERT_TRUE(it);
		any val;
		ASSERT_TRUE(it(val));
		ASSERT_EQ(10, val.get<int>());
		ASSERT_TRUE(it(val));
		ASSERT_EQ(20, val.get<int>());
		ASSERT_TRUE(it(val));
		ASSERT_EQ(30, val.get<int>());
		ASSERT_TRUE(it(val));
		ASSERT_EQ(40, val.get<int>());
		ASSERT_FALSE(it(val));
		ASSERT_FALSE(it(val));
	}
	{
		any test = ttl::any(vec);
		auto it = test.iterate();
		ASSERT_TRUE(it);
		any val;
		ASSERT_TRUE(it(val));
		ASSERT_EQ(10, val.get<int>());
		ASSERT_TRUE(it(val));
		ASSERT_EQ(20, val.get<int>());
		ASSERT_TRUE(it(val));
		ASSERT_EQ(30, val.get<int>());
		ASSERT_TRUE(it(val));
		ASSERT_EQ(40, val.get<int>());
		ASSERT_FALSE(it(val));
		ASSERT_FALSE(it(val));
	}
}
