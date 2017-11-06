#include <gtest/gtest.h>
#include <ttl/any.h>

using thalhammer::any;


struct A {
	virtual ~A() {}
	virtual void test() {
		std::cout<<"A"<<std::endl;
	}
	int x = 0;
};
struct B {
	int y;
	virtual void test2() {

	}
};
struct C : A, B {
	virtual void test() {
		std::cout<<"C"<<std::endl;
	}
	int y = 1;
};
struct D : C {
	virtual void test2() {

	}
};

struct E {

};
struct F : E {

};

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

#ifdef __GLIBCXX__
TEST(AnyTest, UpCast) {
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
}
#endif