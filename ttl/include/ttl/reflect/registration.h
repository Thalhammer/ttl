#pragma once
#include "builder.h"
#include "helper.h"
#include "../cxx11_helpers.h"
#include <map>

namespace ttl
{
    namespace reflect
    {
        class registration {
            std::map<std::string, std::shared_ptr<class_info>> classmap;
            std::map<std::string, std::vector<std::shared_ptr<method_info>>> methodmap;
            static registration& get_instance();
            static void register_class(std::unique_ptr<class_info> ptr) {
                auto name = ptr->get_name();
                get_instance().classmap[name] = std::shared_ptr<class_info>(ptr.release());
            }
        public:
            template<typename T>
            class builder {
                friend class registration;
                std::unique_ptr<reflect::builder<T>> mbuilder;
            public:
                builder(const std::string& name, const std::vector<any>& attributes = {})
                    : mbuilder(ttl::make_unique<reflect::builder<T>>(name.empty()?type::create<T>().pretty_name():name, attributes))
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
                builder& constructor(const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}, const std::vector<any>& attributes = {}) {
                    if(mbuilder)
                        mbuilder->template constructor<Args...>(paramnames, defaultvals, attributes);
                    return *this;
                }

                builder& method(const std::string& name, function fn, const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}, const std::vector<any>& attributes = {}) {
                    if(mbuilder)
                        mbuilder->method(name, fn, paramnames, defaultvals, attributes);
                    return *this;
                }

                template<typename TPtr>
                builder& field(const std::string& name, TPtr ptr, const std::vector<any>& attributes = {}) {
                    if(mbuilder)
                        mbuilder->field(name, ptr, attributes);
                    return *this;
                }

                template<typename TBase>
                builder& base() {
                    if(mbuilder)
                        mbuilder->template base<TBase>();
                    return *this;
                }
            };
            
            template<typename T>
            static registration::builder<T> class_(const std::string& name="") {
                return std::move(builder<T>(name));
            }

            static void method(const std::string& name, function fn, const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}, const std::vector<any>& attributes = {}) {
                get_instance().methodmap[name].push_back(std::shared_ptr<method_info>(new method_info(nullptr, name, fn, paramnames, defaultvals, attributes)));
            }
            
            static std::shared_ptr<class_info> get_class(const std::string& name) {
                auto it = get_instance().classmap.find(name);
                if(it == get_instance().classmap.cend()) return nullptr;
                return it->second;
            }

            template<typename T>
            static std::shared_ptr<class_info> get_class() {
                return get_class(type::create<T>().pretty_name());
            }

            static std::vector<std::shared_ptr<method_info>> get_method(const std::string& name) {
                auto it = get_instance().methodmap.find(name);
                if(it == get_instance().methodmap.cend()) return {};
                return it->second;
            }
        };
    }
}

#define TTL_REFLECT_IMPL2(x,y) \
struct ttl_reflect_init##y { \
    ttl_reflect_init##y () { \
        using namespace ttl::reflect; \
        x \
    } \
}; \
static struct ttl_reflect_init##y ttl_reflect_init_constructor##y;
#define TTL_REFLECT_IMPL(x,y) TTL_REFLECT_IMPL2(x, y)
#define TTL_REFLECT(x) TTL_REFLECT_IMPL(x, __COUNTER__)

#define TTL_REFLECT_REGISTRATION_IMPL() \
ttl::reflect::registration& ttl::reflect::registration::get_instance() { \
    static ttl::reflect::registration instance; \
    return instance; \
}
