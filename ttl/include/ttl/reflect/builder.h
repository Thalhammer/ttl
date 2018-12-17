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
                constructor_info info(*res, static_cast<T(*)(Args...)>([](Args... args) -> T {
                    return T(args...);
                }));
                if(paramnames.size() > info.params.size())
                    throw std::logic_error("Number of parameter names exceeds number of parameters");
                if(defaultvals.size() > info.params.size())
                    throw std::logic_error("Number of default values exceeds number of parameters");
                for(size_t i=0; i<paramnames.size(); i++) {
                    info.params[i].name = paramnames[i];
                }
                size_t offset = info.params.size() - defaultvals.size();
                for(size_t i=0; i<defaultvals.size(); i++) {
                    info.params[i + offset].default_val = defaultvals[i];
                }
                res->constructors.push_back(info);
                return *this;
            }

            builder& method(const std::string& name, function fn, const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}) {
                method_info info(res.get(), fn);
                info.name = name;
                if(paramnames.size() > info.params.size())
                    throw std::logic_error("Number of parameter names exceeds number of parameters");
                if(defaultvals.size() > info.params.size())
                    throw std::logic_error("Number of default values exceeds number of parameters");
                for(size_t i=0; i<paramnames.size(); i++) {
                    info.params[i].name = paramnames[i];
                }
                size_t offset = info.params.size() - defaultvals.size();
                for(size_t i=0; i<defaultvals.size(); i++) {
                    info.params[i + offset].default_val = defaultvals[i];
                }
                res->methods.push_back(info);
                return *this;
            }

            template<typename TPtr>
            builder& field(const std::string& name, TPtr ptr) {
                res->fields.push_back(field_info(res.get(), name, ptr));
                return *this;
            }

            std::unique_ptr<class_info> build() {
                return std::move(res);
            }
        };
    }
}
