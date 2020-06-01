#include <gtest/gtest.h>

#include "ttl/promise.h"

using ttl::promise;

TEST(PromiseTest, ReadyReject) {
    auto t = promise<int>::reject(nullptr);
    ASSERT_EQ(t->get_state(), promise<int>::state::rejected);
    bool executed = false;
    t->error([&](std::exception_ptr e) {
        ASSERT_EQ(e, nullptr);
        executed = true;
    });
    ASSERT_TRUE(executed);
    executed = false;
    t->then([&](int) {
        executed = true;
    });
    ASSERT_FALSE(executed);
}

TEST(PromiseTest, ReadyResolve) {
    auto t = promise<int>::resolve(10);
    ASSERT_EQ(t->get_state(), promise<int>::state::resolved);
    bool executed = false;
    t->error([&](std::exception_ptr) {
        executed = true;
    });
    ASSERT_FALSE(executed);
    executed = false;
    t->then([&](int v) {
        ASSERT_EQ(v, 10);
        executed = true;
    });
    ASSERT_TRUE(executed);
}

TEST(PromiseTest, Resolve) {
    promise<int>::resolve_fn_t mresolve;
    promise<int>::reject_fn_t mreject;
    auto t = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
        mresolve = resolve;
        mreject = reject;
    });
    ASSERT_EQ(t->get_state(), promise<int>::state::pending);
    bool error_executed = false;
    bool resolve_executed = false;
    t->then([&](int v) {
        ASSERT_EQ(v, 10);
        resolve_executed = true;
    }, [&](std::exception_ptr) {
        error_executed = true;
    });
    ASSERT_FALSE(error_executed);
    ASSERT_FALSE(resolve_executed);

    mresolve(10);

    ASSERT_EQ(t->get_state(), promise<int>::state::resolved);
    ASSERT_FALSE(error_executed);
    ASSERT_TRUE(resolve_executed);
}

TEST(PromiseTest, Reject) {
    promise<int>::resolve_fn_t mresolve;
    promise<int>::reject_fn_t mreject;
    auto t = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
        mresolve = resolve;
        mreject = reject;
    });
    ASSERT_EQ(t->get_state(), promise<int>::state::pending);
    bool error_executed = false;
    bool resolve_executed = false;
    t->then([&](int v) {
        ASSERT_EQ(v, 10);
        resolve_executed = true;
    }, [&](std::exception_ptr e) {
        ASSERT_EQ(e, nullptr);
        error_executed = true;
    });
    ASSERT_FALSE(error_executed);
    ASSERT_FALSE(resolve_executed);

    mreject(nullptr);

    ASSERT_EQ(t->get_state(), promise<int>::state::rejected);
    ASSERT_TRUE(error_executed);
    ASSERT_FALSE(resolve_executed);
}

TEST(PromiseTest, RaceReject) {
    {
        promise<int>::resolve_fn_t mresolve1, mresolve2;
        promise<int>::reject_fn_t mreject1, mreject2;
        auto t1 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve1 = resolve;
            mreject1 = reject;
        });
        auto t2 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve2 = resolve;
            mreject2 = reject;
        });
        auto t = promise<int>::race({ t1, t2 });
        ASSERT_EQ(t->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        bool error_executed = false;
        bool resolve_executed = false;
        t->then([&](int v) {
            ASSERT_EQ(v, 10);
            resolve_executed = true;
        }, [&](std::exception_ptr e) {
            ASSERT_EQ(e, nullptr);
            error_executed = true;
        });
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mreject1(nullptr);

        ASSERT_EQ(t->get_state(), promise<int>::state::rejected);
        ASSERT_EQ(t1->get_state(), promise<int>::state::rejected);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        ASSERT_TRUE(error_executed);
        ASSERT_FALSE(resolve_executed);
    }
    {
        promise<int>::resolve_fn_t mresolve1, mresolve2;
        promise<int>::reject_fn_t mreject1, mreject2;
        auto t1 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve1 = resolve;
            mreject1 = reject;
        });
        auto t2 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve2 = resolve;
            mreject2 = reject;
        });
        auto t = promise<int>::race({ t1, t2 });
        ASSERT_EQ(t->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        bool error_executed = false;
        bool resolve_executed = false;
        t->then([&](int v) {
            ASSERT_EQ(v, 10);
            resolve_executed = true;
        }, [&](std::exception_ptr e) {
            ASSERT_EQ(e, nullptr);
            error_executed = true;
        });
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mreject2(nullptr);

        ASSERT_EQ(t->get_state(), promise<int>::state::rejected);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::rejected);
        ASSERT_TRUE(error_executed);
        ASSERT_FALSE(resolve_executed);
    }
}

TEST(PromiseTest, RaceResolve) {
    {
        promise<int>::resolve_fn_t mresolve1, mresolve2;
        promise<int>::reject_fn_t mreject1, mreject2;
        auto t1 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve1 = resolve;
            mreject1 = reject;
        });
        auto t2 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve2 = resolve;
            mreject2 = reject;
        });
        auto t = promise<int>::race({ t1, t2 });
        ASSERT_EQ(t->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        bool error_executed = false;
        bool resolve_executed = false;
        t->then([&](int v) {
            ASSERT_EQ(v, 10);
            resolve_executed = true;
        }, [&](std::exception_ptr e) {
            ASSERT_EQ(e, nullptr);
            error_executed = true;
        });
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mresolve1(10);

        ASSERT_EQ(t->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t1->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        ASSERT_FALSE(error_executed);
        ASSERT_TRUE(resolve_executed);
    }
    {
        promise<int>::resolve_fn_t mresolve1, mresolve2;
        promise<int>::reject_fn_t mreject1, mreject2;
        auto t1 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve1 = resolve;
            mreject1 = reject;
        });
        auto t2 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve2 = resolve;
            mreject2 = reject;
        });
        auto t = promise<int>::race({ t1, t2 });
        ASSERT_EQ(t->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        bool error_executed = false;
        bool resolve_executed = false;
        t->then([&](int v) {
            ASSERT_EQ(v, 10);
            resolve_executed = true;
        }, [&](std::exception_ptr e) {
            ASSERT_EQ(e, nullptr);
            error_executed = true;
        });
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mresolve2(10);

        ASSERT_EQ(t->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::resolved);
        ASSERT_FALSE(error_executed);
        ASSERT_TRUE(resolve_executed);
    }
}

TEST(PromiseTest, RaceBoth) {
    {
        promise<int>::resolve_fn_t mresolve1, mresolve2;
        promise<int>::reject_fn_t mreject1, mreject2;
        auto t1 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve1 = resolve;
            mreject1 = reject;
        });
        auto t2 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve2 = resolve;
            mreject2 = reject;
        });
        auto t = promise<int>::race({ t1, t2 });
        ASSERT_EQ(t->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        bool error_executed = false;
        bool resolve_executed = false;
        t->then([&](int v) {
            ASSERT_EQ(v, 10);
            resolve_executed = true;
        }, [&](std::exception_ptr e) {
            ASSERT_EQ(e, nullptr);
            error_executed = true;
        });
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mresolve1(10);

        ASSERT_EQ(t->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t1->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        ASSERT_FALSE(error_executed);
        ASSERT_TRUE(resolve_executed);
        resolve_executed = false;

        mreject2(nullptr);

        ASSERT_EQ(t->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t1->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t2->get_state(), promise<int>::state::rejected);
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);
    }
    {
        promise<int>::resolve_fn_t mresolve1, mresolve2;
        promise<int>::reject_fn_t mreject1, mreject2;
        auto t1 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve1 = resolve;
            mreject1 = reject;
        });
        auto t2 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve2 = resolve;
            mreject2 = reject;
        });
        auto t = promise<int>::race({ t1, t2 });
        ASSERT_EQ(t->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        bool error_executed = false;
        bool resolve_executed = false;
        t->then([&](int v) {
            ASSERT_EQ(v, 10);
            resolve_executed = true;
        }, [&](std::exception_ptr e) {
            ASSERT_EQ(e, nullptr);
            error_executed = true;
        });
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mresolve2(10);

        ASSERT_EQ(t->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::resolved);
        ASSERT_FALSE(error_executed);
        ASSERT_TRUE(resolve_executed);
        resolve_executed = false;

        mreject1(nullptr);

        ASSERT_EQ(t->get_state(), promise<int>::state::resolved);
        ASSERT_EQ(t1->get_state(), promise<int>::state::rejected);
        ASSERT_EQ(t2->get_state(), promise<int>::state::resolved);
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);
    }
}

TEST(PromiseTest, All) {
    {
        promise<int>::resolve_fn_t mresolve1, mresolve2;
        promise<int>::reject_fn_t mreject1, mreject2;
        auto t1 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve1 = resolve;
            mreject1 = reject;
        });
        auto t2 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve2 = resolve;
            mreject2 = reject;
        });
        auto t = promise<int>::all({ t1, t2 });
        ASSERT_EQ(t->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        bool error_executed = false;
        bool resolve_executed = false;
        t->then([&](std::vector<int> v) {
            ASSERT_EQ(v.size(), 2);
            ASSERT_EQ(v[0], 1);
            ASSERT_EQ(v[1], 2);
            resolve_executed = true;
        }, [&](std::exception_ptr e) {
            ASSERT_EQ(e, nullptr);
            error_executed = true;
        });
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mresolve1(1);
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mresolve2(2);
        ASSERT_FALSE(error_executed);
        ASSERT_TRUE(resolve_executed);
    }
    {
        promise<int>::resolve_fn_t mresolve1, mresolve2;
        promise<int>::reject_fn_t mreject1, mreject2;
        auto t1 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve1 = resolve;
            mreject1 = reject;
        });
        auto t2 = promise<int>::create([&](promise<int>::resolve_fn_t resolve, promise<int>::reject_fn_t reject){
            mresolve2 = resolve;
            mreject2 = reject;
        });
        auto t = promise<int>::all({ t1, t2 });
        ASSERT_EQ(t->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t1->get_state(), promise<int>::state::pending);
        ASSERT_EQ(t2->get_state(), promise<int>::state::pending);
        bool error_executed = false;
        bool resolve_executed = false;
        t->then([&](std::vector<int> v) {
            ASSERT_EQ(v.size(), 2);
            ASSERT_EQ(v[0], 1);
            ASSERT_EQ(v[1], 2);
            resolve_executed = true;
        }, [&](std::exception_ptr e) {
            ASSERT_EQ(e, nullptr);
            error_executed = true;
        });
        ASSERT_FALSE(error_executed);
        ASSERT_FALSE(resolve_executed);

        mreject1(nullptr);
        ASSERT_TRUE(error_executed);
        ASSERT_FALSE(resolve_executed);
    }
}
