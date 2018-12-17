#pragma once
#include "builder.h"
#include <map>

namespace ttl
{
    namespace reflect
    {
        class registration {
            static std::map<std::string, std::shared_ptr<class_info>>& get_classmap();
            static void register_class(std::unique_ptr<class_info> ptr) {
                auto name = ptr->get_name();
                get_classmap()[name] = std::shared_ptr<class_info>(ptr.release());
            }
        public:
            template<typename T>
            class builder {
                friend class registration;
                std::unique_ptr<reflect::builder<T>> mbuilder;
            public:
                builder(const std::string& name)
                    : mbuilder(std::make_unique<reflect::builder<T>>(name.empty()?type::create<T>().pretty_name():name))
                {}
                builder(builder&& other)
                    : mbuilder(std::move(other.mbuilder))
                {}
                ~builder()
                {
                    if(mbuilder)
                        registration::register_class(mbuilder->build());
                }

                template<typename... Args>
                builder& constructor(const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}) {
                    if(mbuilder)
                        mbuilder->template constructor<Args...>();
                    return *this;
                }

                builder& method(const std::string& name, function fn, const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}) {
                    if(mbuilder)
                        mbuilder->method(name, fn, paramnames, defaultvals);
                    return *this;
                }

                template<typename TPtr>
                builder& field(const std::string& name, TPtr ptr) {
                    if(mbuilder)
                        mbuilder->field(name, ptr);
                    return *this;
                }
            };
            template<typename T>
            static registration::builder<T> class_(const std::string& name="") {
                return std::move(builder<T>(name));
            }

            static std::shared_ptr<class_info> get_class(const std::string& name) {
                auto it = get_classmap().find(name);
                if(it == get_classmap().cend()) return nullptr;
                return it->second;
            }

            template<typename T>
            static std::shared_ptr<class_info> get_class() {
                return get_class(type::create<T>().pretty_name());
            }
        };
    }
}

#define TTL_REFLECT(x) \
struct ttl_reflect_init { \
    ttl_reflect_init() { \
        using namespace ttl::reflect; \
        x \
    } \
}; \
static struct ttl_reflect_init ttl_reflect_init_constructor;

#define TTL_REFLECT_REGISTRATION_IMPL() \
std::map<std::string, std::shared_ptr<ttl::reflect::class_info>>& ttl::reflect::registration::get_classmap() { \
    static std::map<std::string, std::shared_ptr<ttl::reflect::class_info>> classes; \
    return classes; \
};
