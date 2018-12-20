#pragma once
#include <functional>
#include "../any.h"

namespace ttl
{
    namespace reflect
    {
        template<typename T>
        class builder;
        class class_info;
        class field_info {
            const class_info* pclass;
            std::string name;
            type ftype;
            bool _static;
            std::function<void(any&, any)> setter;
            std::function<any(any&)> getter;
            std::vector<any> attributes;
            template<typename T> friend class builder;
            template<typename T>
            field_info(const class_info* parent, std::string pname, T* field, const std::vector<any>& pattributes = {})
                : pclass(parent), name(pname), ftype(type::create<T>()), attributes(pattributes)
            {
                _static = true;
                setter = [field](any&, any val) {
                    *field = val.get<T>();
                };
                getter = [field](any&) -> any{
                    return any(*field);
                };
            }
            template<typename T, typename Class>
            field_info(const class_info* parent, std::string pname, T Class::*field, const std::vector<any>& pattributes = {})
                : pclass(parent), name(pname), ftype(type::create<T>()), attributes(pattributes)
            {
                _static = false;
                setter = [field](any& instance, any val) {
                    (instance.get_pointer<Class>()->*field) = val.get<T>();
                };
                getter = [field](any& instance) -> any{
                    return any((instance.get_pointer<Class>()->*field));
                };
            }
        public:
            const class_info* get_declaring_class() const noexcept {
                return pclass;
            }

            const std::string& get_name() const noexcept {
                return name;
            }

            const type& get_type() const noexcept {
                return ftype;
            }

            bool is_static() const noexcept {
                return _static;
            }

            const std::vector<any> get_attributes() const noexcept {
                return attributes;
            }

            any get() const {
                if(!is_static()) throw std::logic_error("missing instance to access non static field");
                any empty;
                return getter(empty);
            }

            void set(any value) const {
                if(!is_static()) throw std::logic_error("missing instance to access non static field");
                any empty;
                setter(empty, value);
            }

            any get(any& instance) const {
                return getter(instance);
            }

            void set(any& instance, any value) const {
                setter(instance, value);
            }
        };
    }
}
