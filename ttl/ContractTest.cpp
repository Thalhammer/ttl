#include <gtest/gtest.h>

#include <ttl/contract.h>

using namespace thalhammer;

TEST(ContractTest, Checked) {
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
    ASSERT_NO_THROW(eq1());
    ASSERT_THROW(eq2(), contract_failed);
    ASSERT_THROW(eq3(), contract_failed);
}
