#include <gtest/gtest.h>
#include "include/ttl/reflect/registration.h"

TTL_REFLECT_REGISTRATION_IMPL()

namespace ReflectionTest {
    struct testbase {
        int test_val2;
    };

    struct test : testbase {
        int add(int a = 10, int b = 20) { return a + b; }

        int test_val;
    };
}

TTL_REFLECT(
    using namespace ReflectionTest;
    registration::class_<testbase>()
        .constructor<>()
        .field("test_val2", &testbase::test_val2);
    registration::class_<test>()
        .base<testbase>()
        .constructor<>()
        .method("add", &test::add, {"a","b"}, {10, 20})
        .field("test_val", &test::test_val);
    registration::method("std::to_string", select_overload<std::string(int)>(&std::to_string), { "value" });
)

std::shared_ptr<const ttl::reflect::class_info> get_info() {
    using namespace ReflectionTest;
    return ttl::reflect::registration::get_class<test>();
}

TEST(ReflectionTest, BuildClassInfo) {
	auto info = get_info();
}

TEST(ReflectionTest, ExamineMethods) {
	auto info = get_info();

    auto meths = info->get_methods();
    ASSERT_EQ(meths.size(), 1);

    ASSERT_EQ(meths[0].get_name(), "add");
    ASSERT_EQ(meths[0].get_declaring_class(), info.get());
    ASSERT_FALSE(meths[0].is_static());
    auto params = meths[0].get_parameters();
    ASSERT_EQ(params.size(), 2);

    ASSERT_EQ(params[0].get_name(), "a");
    ASSERT_EQ(params[0].get_index(), 0);
    ASSERT_EQ(params[0].get_type(), ttl::type::create<int>());
    ASSERT_TRUE(params[0].has_default_value());
    ASSERT_TRUE(params[0].get_default_value().is_type<int>());
    ASSERT_EQ(params[0].get_default_value().get<int>(), 10);

    ASSERT_EQ(params[1].get_name(), "b");
    ASSERT_EQ(params[1].get_index(), 1);
    ASSERT_EQ(params[1].get_type(), ttl::type::create<int>());
    ASSERT_TRUE(params[1].has_default_value());
    ASSERT_TRUE(params[1].get_default_value().is_type<int>());
    ASSERT_EQ(params[1].get_default_value().get<int>(), 20);
}

TEST(ReflectionTest, InvokeMember) {
    using namespace ReflectionTest;
	auto info = get_info();

    auto meths = info->get_methods();
    ASSERT_EQ(meths.size(), 1);

    ttl::any instance(test{});
    auto res = meths[0].invoke(instance, {30, 30});
    ASSERT_TRUE(res.has_value());
    ASSERT_TRUE(res.value().is_type<int>());
    ASSERT_EQ(res.value().get<int>(), 60);
}

TEST(ReflectionTest, InvokeMemberDefaultArgs) {
    using namespace ReflectionTest;
	auto info = get_info();

    auto meths = info->get_methods();
    ASSERT_EQ(meths.size(), 1);

    ttl::any instance(test{});
    auto res = meths[0].invoke(instance, {});
    ASSERT_TRUE(res.has_value());
    ASSERT_TRUE(res.value().is_type<int>());
    ASSERT_EQ(res.value().get<int>(), 30);
}

TEST(ReflectionTest, ExamineFields) {
    using namespace ReflectionTest;
	auto info = get_info();

    auto fields = info->get_fields();
    ASSERT_EQ(fields.size(), 1);

    ASSERT_EQ(fields[0].get_name(), "test_val");
    ASSERT_EQ(fields[0].get_declaring_class(), info.get());
    ASSERT_FALSE(fields[0].is_static());
    ASSERT_EQ(fields[0].get_type(), ttl::type::create<int>());

    test t;
    t.test_val = 10;
    ttl::any instance(t);
    auto val = fields[0].get(instance);
    ASSERT_TRUE(val.valid());
    ASSERT_TRUE(instance.is_type<test>());
    ASSERT_EQ(val.get<int>(), 10);
    fields[0].set(instance, 20);
    val = fields[0].get(instance);
    ASSERT_EQ(val.get<int>(), 20);
}

TEST(ReflectionTest, Constructor) {
    using namespace ReflectionTest;
	auto info = get_info();

    auto constructors = info->get_constructors();
    ASSERT_EQ(constructors.size(), 1);
    auto instance = constructors[0].invoke({});
    ASSERT_TRUE(instance.has_value());
    ASSERT_TRUE(instance.value().is_type<test>());
}


TEST(ReflectionTest, GlobalMethod) {
	auto info = ttl::reflect::registration::get_method("std::to_string");

    ASSERT_EQ(info.size(), 1);
    auto method = info[0];
    ASSERT_EQ(method->get_name(), "std::to_string");
    ASSERT_EQ(method->get_declaring_class(), nullptr);
    ASSERT_TRUE(method->is_static());
    auto params = method->get_parameters();
    ASSERT_EQ(params.size(), 1);
    ASSERT_EQ(&params[0].get_declaring_method(), method.get());
    ASSERT_EQ(params[0].get_index(), 0);
    ASSERT_EQ(params[0].get_name(), "value");
    ASSERT_EQ(params[0].get_type(), ttl::type::create<int>());
    ASSERT_FALSE(params[0].has_default_value());

    auto res = method->invoke({ 10 });
    ASSERT_TRUE(res.has_value());
    ASSERT_TRUE(res.value().is_type<std::string>());
    ASSERT_EQ(res->get<std::string>(), "10");
}

TEST(ReflectionTest, Bases) {
    using namespace ReflectionTest;
	auto info = get_info();

    auto bases = info->get_base_classes();
    ASSERT_EQ(bases.size(), 1);
    test t;
    ttl::any a = bases[0].convert(&t);
    ASSERT_TRUE(!a.empty());
    ASSERT_TRUE(a.is_type<testbase*>());
    ASSERT_EQ(a.get<testbase*>(), static_cast<testbase*>(&t));
}