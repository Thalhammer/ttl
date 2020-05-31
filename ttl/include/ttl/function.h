#pragma once
#include <memory>
#include <functional>
#include "any.h"
#include "optional.h"
#include "cxx11_helpers.h"

namespace ttl
{
    namespace detail
    {
        template <typename... Args>
        struct get_types_impl;
        template <typename T, typename... Args>
        struct get_types_impl<T, Args...>
        {
            static void get_types(std::vector<type> &vect)
            {
                vect.push_back(type::create<T>());
                get_types_impl<Args...>::get_types(vect);
            }
        };
        template <>
        struct get_types_impl<>
        {
            static void get_types(std::vector<type> &vect)
            {
                (void)vect;
            }
        };
        template <typename Result>
        static Result call_detail(std::function<Result()> fn, const std::vector<any> &a, size_t i)
        {
            if (a.size() < i)
                throw std::out_of_range("not enough parameters");
            return fn();
        }
        template <typename Result, typename Arg1>
        static Result call_detail(std::function<Result(Arg1)> fn, const std::vector<any> &a, size_t i)
        {
            auto nfn = [a, fn, i]() {
                auto arg1 = a[i].get<Arg1>();
                return fn(arg1);
            };
            return call_detail<Result>(nfn, a, i + 1);
        }
        template <typename Result, typename Arg1, typename Arg2, typename... Args>
        static Result call_detail(std::function<Result(Arg1, Arg2, Args...)> fn, const std::vector<any> &a, size_t i)
        {
            auto nfn = [a, fn, i](Args... args) {
                auto arg1 = a[i].get<Arg1>();
                auto arg2 = a[i + 1].get<Arg2>();
                return fn(arg1, arg2, args...);
            };
            return call_detail<Result, Args...>(nfn, a, i + 2);
        }
    } // namespace detail
    class function final
    {
        struct data_base
        {
            data_base() = default;
            data_base(const data_base&) = default;
            data_base(data_base&&) = default;
            virtual ~data_base() {}
            virtual bool requires_instance() const noexcept = 0;
            virtual bool requires_modifiable_instance() const noexcept = 0;
            virtual optional<type> get_instance_type() const = 0;
            virtual type get_return_type() const = 0;
            virtual std::vector<type> get_parameter_types() const = 0;
            virtual optional<any> invoke_dynamic(any &instance, const std::vector<any> &params) const = 0;
            virtual std::unique_ptr<data_base> clone() const = 0;
        };
        template <typename Ret, typename... Args>
        struct data : data_base
        {
            data(std::function<Ret(Args...)> pfn)
                : fn_ptr(pfn)
            {
            }

            bool requires_instance() const noexcept override
            {
                return false;
            }

            bool requires_modifiable_instance() const noexcept override
            {
                return false;
            }

            optional<type> get_instance_type() const override
            {
                return {};
            }

            type get_return_type() const override
            {
                return type::create<Ret>();
            }

            std::vector<type> get_parameter_types() const override
            {
                std::vector<type> types;
                detail::get_types_impl<Args...>::get_types(types);
                return types;
            }

            template <typename U = Ret>
            typename std::enable_if<std::is_same<U, void>::value, optional<any>>::type
            invoke_dynamic_impl(const std::vector<any> &params) const
            {
                detail::call_detail<Ret, Args...>(fn_ptr, params, 0);
                return {};
            }

            template <typename U = Ret>
            typename std::enable_if<!std::is_same<U, void>::value, optional<any>>::type
            invoke_dynamic_impl(const std::vector<any> &params) const
            {
                return {detail::call_detail<Ret, Args...>(fn_ptr, params, 0)};
            }

            optional<any> invoke_dynamic(any &instance, const std::vector<any> &params) const override
            {
                (void)instance;
                auto types = this->get_parameter_types();
                if (params.size() != types.size())
                    throw std::logic_error("wrong parameter count");
                for (size_t i = 0; i < params.size(); i++)
                {
                    if (params[i].type() != types[i])
                    {
                        throw std::runtime_error("invalid type for arg " + std::to_string(i) + " expected: " + types[i].pretty_name() + " got: " + params[i].type().pretty_name());
                    }
                }
                return invoke_dynamic_impl(params);
            }

            std::unique_ptr<data_base> clone() const override
            {
                return ttl::make_unique<data>(*this);
            }

        private:
            std::function<Ret(Args...)> fn_ptr;
        };
        template <typename Class, typename Ret, typename... Args>
        struct data_memberfn : data_base
        {
            data_memberfn(Ret (Class::*pfn)(Args...))
                : fn_ptr(pfn)
            {
            }

            bool requires_instance() const noexcept override
            {
                return true;
            }

            bool requires_modifiable_instance() const noexcept override
            {
                return true;
            }

            optional<type> get_instance_type() const override
            {
                return type::create<Class>();
            }

            type get_return_type() const override
            {
                return type::create<Ret>();
            }

            std::vector<type> get_parameter_types() const override
            {
                std::vector<type> types;
                detail::get_types_impl<Args...>::get_types(types);
                return types;
            }

            template <typename U = Ret>
            typename std::enable_if<std::is_same<U, void>::value, optional<any>>::type
            invoke_dynamic_impl(Class *instance, const std::vector<any> &params) const
            {
                detail::call_detail<Ret, Args...>(std::function<U(Args...)>([instance, this](Args... args) -> U {
                    return (instance->*fn_ptr)(args...);
                }), params, 0);
                return {};
            }

            template <typename U = Ret>
            typename std::enable_if<!std::is_same<U, void>::value, optional<any>>::type
            invoke_dynamic_impl(Class *instance, const std::vector<any> &params) const
            {
                auto res = detail::call_detail<Ret, Args...>(std::function<U(Args...)>([instance, this](Args... args) -> U {
                    return (instance->*fn_ptr)(args...);
                }), params, 0);
                return optional<any>(any(res));
            }

            optional<any> invoke_dynamic(any &instance, const std::vector<any> &params) const override
            {
                auto types = this->get_parameter_types();
                if (params.size() != types.size())
                    throw std::logic_error("wrong parameter count");
                if (instance.empty())
                    throw std::logic_error("method requires instance");
                if (!instance.is_type<Class>())
                    throw std::logic_error("instance type missmatch " + instance.type().pretty_name() + " != " + type::create<Class>().pretty_name());
                if (instance.type().is_const())
                    throw std::logic_error("trying to call nonconst function on const instance");
                for (size_t i = 0; i < params.size(); i++)
                {
                    if (params[i].type() != types[i])
                    {
                        throw std::runtime_error("invalid type for arg " + std::to_string(i) + " expected: " + types[i].pretty_name() + " got: " + params[i].type().pretty_name());
                    }
                }
                return invoke_dynamic_impl(instance.get_pointer<Class>(), params);
                return {};
            }

            std::unique_ptr<data_base> clone() const override
            {
                return ttl::make_unique<data_memberfn>(*this);
            }

        private:
            Ret (Class::*fn_ptr)(Args...);
        };
        template <typename Class, typename Ret, typename... Args>
        struct data_constmemberfn : data_base
        {
            data_constmemberfn(Ret (Class::*pfn)(Args...) const)
                : fn_ptr(pfn)
            {
            }

            bool requires_instance() const noexcept override
            {
                return true;
            }

            bool requires_modifiable_instance() const noexcept override
            {
                return false;
            }

            optional<type> get_instance_type() const override
            {
                return type::create<Class>();
            }

            type get_return_type() const override
            {
                return type::create<Ret>();
            }

            std::vector<type> get_parameter_types() const override
            {
                std::vector<type> types;
                detail::get_types_impl<Args...>::get_types(types);
                return types;
            }

            template <typename U = Ret>
            typename std::enable_if<std::is_same<U, void>::value, optional<any>>::type
            invoke_dynamic_impl(Class *instance, const std::vector<any> &params) const
            {
                detail::call_detail<Ret, Args...>(std::function<U(Args...)>([instance, this](Args... args) -> U {
                    return (instance->*fn_ptr)(args...);
                }), params, 0);
                return {};
            }

            template <typename U = Ret>
            typename std::enable_if<!std::is_same<U, void>::value, optional<any>>::type
            invoke_dynamic_impl(Class *instance, const std::vector<any> &params) const
            {
                auto res = detail::call_detail<Ret, Args...>(std::function<U(Args...)>([instance, this](Args... args) -> U {
                    return (instance->*fn_ptr)(args...);
                }), params, 0);
                return optional<any>(any(res));
            }

            optional<any> invoke_dynamic(any &instance, const std::vector<any> &params) const override
            {
                auto types = this->get_parameter_types();
                if (params.size() != types.size())
                    throw std::logic_error("wrong parameter count");
                if (instance.empty())
                    throw std::logic_error("method requires instance");
                if (!instance.is_type<Class>())
                    throw std::logic_error("instance type missmatch " + instance.type().pretty_name() + " != " + type::create<Class>().pretty_name());
                for (size_t i = 0; i < params.size(); i++)
                {
                    if (params[i].type() != types[i])
                    {
                        throw std::runtime_error("invalid type for arg " + std::to_string(i) + " expected: " + types[i].pretty_name() + " got: " + params[i].type().pretty_name());
                    }
                }
                return invoke_dynamic_impl(instance.get_pointer<Class>(), params);
                return {};
            }

            std::unique_ptr<data_base> clone() const override
            {
                return ttl::make_unique<data_constmemberfn>(*this);
            }

        private:
            Ret (Class::*fn_ptr)(Args...) const;
        };
        std::unique_ptr<data_base> fn;

    public:
        template <typename Ret, typename... Args>
        function(std::function<Ret(Args...)> pfn)
            : fn(ttl::make_unique<data<Ret, Args...>>(pfn))
        {
        }

        template <typename Ret, typename... Args>
        function(Ret (*ptr)(Args...))
            : fn(ttl::make_unique<data<Ret, Args...>>(ptr))
        {
        }

        template <typename Class, typename Ret, typename... Args>
        function(Ret (Class::*ptr)(Args...))
            : fn(ttl::make_unique<data_memberfn<Class, Ret, Args...>>(ptr))
        {
        }

        template <typename Class, typename Ret, typename... Args>
        function(Ret (Class::*ptr)(Args...) const)
            : fn(ttl::make_unique<data_constmemberfn<Class, Ret, Args...>>(ptr))
        {
        }

        function(const function &other)
            : fn(other.fn->clone())
        {}

        function& operator=(const function& other) {
            fn = other.fn->clone();
            return *this;
        }

        bool requires_instance() const noexcept
        {
            return fn->requires_instance();
        }

        bool requires_modifiable_instance() const noexcept
        {
            return fn->requires_modifiable_instance();
        }

        optional<type> get_instance_type() const
        {
            return fn->get_instance_type();
        }

        type get_return_type() const
        {
            return fn->get_return_type();
        }

        std::vector<type> get_parameter_types() const
        {
            return fn->get_parameter_types();
        }

        optional<any> invoke_dynamic(const std::vector<any> &params) const
        {
            any empty;
            return fn->invoke_dynamic(empty, params);
        }

        optional<any> invoke_dynamic(any &instance, const std::vector<any> &params) const
        {
            return fn->invoke_dynamic(instance, params);
        }

        optional<any> invoke() const
        {
            return invoke_dynamic({});
        }

        optional<any> invoke(any arg1) const
        {
            return invoke_dynamic({arg1});
        }
    };
} // namespace ttl
