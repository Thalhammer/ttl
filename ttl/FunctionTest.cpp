#include <gtest/gtest.h>
#include "include/ttl/function.h"

using ttl::function;
using ttl::type;

namespace FunctionTest {
    void test1() {}
    void test2(const std::string&) {}
    std::string test3() { return ""; }
    std::string test4(const std::string& test) { return test + " world"; }

    struct test {
        int member_method(int i) { return i; }
        static void static_method() {}
    };
}

TEST(FunctionTest, FunctionReturnType) {
	function fn(FunctionTest::test1);
    ASSERT_EQ(fn.get_return_type(), type::create<void>());

    fn = function(FunctionTest::test2);
    ASSERT_EQ(fn.get_return_type(), type::create<void>());

    fn = function(FunctionTest::test3);
    ASSERT_NE(fn.get_return_type(), type::create<void>());
    ASSERT_EQ(fn.get_return_type(), type::create<std::string>());
}

TEST(FunctionTest, FunctionParameterType) {
	function fn(FunctionTest::test1);
    ASSERT_TRUE(fn.get_parameter_types().empty());
 
    fn = function(FunctionTest::test2);
    auto params = fn.get_parameter_types();
    ASSERT_FALSE(params.empty());
    ASSERT_EQ(params.size(), 1);
    ASSERT_EQ(params[0], type::create<const std::string&>());

    fn = function(FunctionTest::test3);
    ASSERT_TRUE(fn.get_parameter_types().empty());
}

TEST(FunctionTest, Invoke) {
	function fn(FunctionTest::test4);

    auto res = fn.invoke(std::string("Hello"));
    ASSERT_TRUE(res.has_value());
    ASSERT_EQ(res.value().type(), type::create<std::string>());
    ASSERT_EQ(res.value().get<std::string>(), "Hello world");
}

TEST(FunctionTest, InvokeVoid) {
	function fn(FunctionTest::test1);

    auto res = fn.invoke();
    ASSERT_FALSE(res.has_value());
}

TEST(FunctionTest, InvokeInvalidArgs) {
	function fn(FunctionTest::test1);

    ASSERT_THROW(fn.invoke({ 10 }), std::logic_error);
}

TEST(FunctionTest, ClassStaticMethod) {
	function fn(&FunctionTest::test::static_method);

    fn.invoke();
    ASSERT_FALSE(fn.requires_instance());
}

TEST(FunctionTest, ClassMemberMethod) {
	function fn(&FunctionTest::test::member_method);

    ASSERT_TRUE(fn.requires_instance());

    ttl::any instance(FunctionTest::test{});
    auto res = fn.invoke_dynamic(instance, {10});
    ASSERT_TRUE(res.value().is_type<int>());
    ASSERT_EQ(res.value().get<int>(), 10);
}

TEST(FunctionTest, Copy) {
	function fn(&FunctionTest::test::member_method);

    function fn2 = fn;
}