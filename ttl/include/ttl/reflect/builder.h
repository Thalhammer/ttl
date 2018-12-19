#pragma once
#include "class_info.h"

namespace ttl
{
    namespace reflect
    {
        template<typename T>
        class builder {
            std::unique_ptr<class_info> res;
        public:
            builder(std::string name)
                : res(new class_info())
            {
                res->name = name;
            }

            template<typename... Args>
            builder& constructor(const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}) {
                res->constructors.push_back(constructor_info(*res, static_cast<T(*)(Args...)>([](Args... args) -> T {
                    return T(args...);
                }), paramnames, defaultvals));
                return *this;
            }

            builder& method(const std::string& name, function fn, const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}) {
                res->methods.push_back(method_info(res.get(), name, fn, paramnames, defaultvals));
                return *this;
            }

            template<typename TPtr>
            builder& field(const std::string& name, TPtr ptr) {
                res->fields.push_back(field_info(res.get(), name, ptr));
                return *this;
            }

            template<typename TBase>
            builder& base() {
                res->base_classes.push_back(base_info(base_info::deduction_helper<TBase>(), base_info::deduction_helper<T>()));
                return *this;
            }

            std::unique_ptr<class_info> build() {
                return std::move(res);
            }
        };
    }
}
