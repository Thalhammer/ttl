#include <gtest/gtest.h>

#include <ttl/contract.h>

using namespace thalhammer;

TEST(ContractTest, CheckedValue) {
    auto eq1 = [](){
        contract_value<int, 100, 1000> num;
        num = 100;
        num = 1000;
        num = 130;
    };
    auto eq2 = [](){
        contract_value<int, 100, 1000> num(1);
    };
    auto eq3 = [](){
        contract_value<int, 100, 1000> num;
        num = 1;
	};
	auto eq4 = [](contract_value<int, 100, 1000> n) {
		ASSERT_EQ(122, n.val());
	};
    ASSERT_NO_THROW(eq1());
    ASSERT_THROW(eq2(), contract_failed);
	ASSERT_THROW(eq3(), contract_failed);
	ASSERT_THROW(eq4(0), contract_failed);
	ASSERT_NO_THROW(eq4(122));
}

TEST(ContractTest, CheckNotNull) {
    auto eq1 = [](){
		int i = 100;
		int x = 200;
		contract_not_null<int> ptr(&i);
		ASSERT_EQ(100, *ptr);
		ptr = &x;
		ASSERT_EQ(200, *ptr);
    };
    auto eq2 = [](){
        contract_not_null<int> ptr(nullptr);
	};
    ASSERT_NO_THROW(eq1());
    ASSERT_THROW(eq2(), contract_failed);
}
