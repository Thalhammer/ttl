#pragma once
#include <functional>
#include <memory>
#include <type_traits>
#include <mutex>
#include "optional.h"

namespace ttl {
    enum class promise_state {
        pending,
        resolved,
        rejected
    };
	template<typename T, typename TError = std::exception_ptr>
	class promise {
    protected:
        promise() {}
        promise(const promise&) = delete;
        promise(promise&&) = delete;
        template<typename U, typename X>
        friend class promise;
	public:
        typedef promise_state state;
		typedef std::shared_ptr<promise> ptr;
        typedef std::function<void(T)> resolve_fn_t;
        typedef std::function<void(TError)> reject_fn_t;
        typedef std::function<void(resolve_fn_t, reject_fn_t)> executor_fn_t;
        
        static ptr create(executor_fn_t fn) {
            auto res = std::shared_ptr<promise>(new promise());
            fn([res](T val){
                res->do_resolve(std::move(val));
            }, [res](TError err){
                res->do_reject(std::move(err));
            });
            return res;
        }

        void then(resolve_fn_t fn) {
            std::unique_lock<std::mutex> lck(m_lck);
            if(m_state == state::pending) {
                m_resolve_list.push_back(fn);
            } else if(m_state == state::resolved) {
                fn(m_value.value());
            }
        }
        void then(resolve_fn_t fn, reject_fn_t err) {
            std::unique_lock<std::mutex> lck(m_lck);
            if(m_state == state::pending) {
                m_resolve_list.push_back(fn);
                m_reject_list.push_back(err);
            } else if(m_state == state::resolved) {
                fn(m_value.value());
            } else if(m_state == state::rejected) {
                err(m_error.value());
            }
        }
        void error(reject_fn_t err) {
            std::unique_lock<std::mutex> lck(m_lck);
            if(m_state == state::pending) {
                m_reject_list.push_back(err);
            } else if(m_state == state::rejected) {
                err(m_error.value());
            }
        }
        state get_state() const {
            std::unique_lock<std::mutex> lck(m_lck);
            return m_state;
        }

        static ptr reject(TError&& err) {
            auto res = std::shared_ptr<promise>(new promise());
            res->m_state = state::rejected;
            res->m_error.emplace(err);
            return res;
        }
        static ptr resolve(T&& val) {
            auto res = std::shared_ptr<promise>(new promise());
            res->m_state = state::resolved;
            res->m_value.emplace(val);
            return res;
        }

        static ptr race(std::vector<ptr> l) {
            auto res = std::shared_ptr<promise>(new promise());
            for(auto& e : l) {
                e->then([res](T val){
                    if(res->get_state() == state::pending) {
                        res->do_resolve(std::move(val));
                    }
                }, [res](TError err){
                    if(res->get_state() == state::pending) {
                        res->do_reject(std::move(err));
                    }
                });
            }
            return res;
        }
        static std::shared_ptr<promise<std::vector<T>>> all(std::vector<ptr> l) {
            struct all_promise : promise<std::vector<T>> {
                std::vector<typename promise<T>::ptr> m_children;
            };
            auto res = std::shared_ptr<all_promise>(new all_promise());
            res->m_children = std::move(l);
            for(auto& e : res->m_children) {
                auto x = e.get();
                e->then([res, x](T){
                    if(res->get_state() == state::pending) {
                        bool ok = true;
                        for(auto& e2 : res->m_children) {
                            if(e2.get() == x) continue;
                            std::unique_lock<std::mutex> lck(e2->m_lck);
                            if(e2->m_state != state::resolved) {
                                ok = false;
                                break;
                            }
                        }
                        if(ok) {
                            std::vector<T> mvect;
                            for(auto& e2 : res->m_children) mvect.push_back(e2->m_value.value());
                            res->do_resolve(std::move(mvect));
                        }
                    }
                }, [res](TError err){
                    if(res->get_state() == state::pending) {
                        res->do_reject(std::move(err));
                    }
                });
            }
            return res;
        }
    protected:
        mutable std::mutex m_lck;
        state m_state = state::pending;
        optional<T> m_value;
        optional<TError> m_error;
        std::vector<resolve_fn_t> m_resolve_list;
        std::vector<reject_fn_t> m_reject_list;

        void do_resolve(T&& val) {
            std::unique_lock<std::mutex> lck(m_lck);
            if(m_state != state::pending)
                throw std::logic_error("promise is not pending");
            m_state = state::resolved;
            m_value.emplace(val);
            for(auto& e : m_resolve_list) {
                try {
                    e(m_value.value());
                } catch(const std::exception&) {}
            }
            m_resolve_list.clear();
            m_reject_list.clear();
        }
        void do_reject(TError&& val) {
            std::unique_lock<std::mutex> lck(m_lck);
            if(m_state != state::pending)
                throw std::logic_error("promise is not pending");
            m_state = state::rejected;
            m_error.emplace(val);
            for(auto& e : m_reject_list) {
                try {
                    e(m_error.value());
                } catch(const std::exception&) {}
            }
            m_resolve_list.clear();
            m_reject_list.clear();
        }
    };
}
