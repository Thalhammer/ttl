#pragma once
#include "../any.h"

namespace ttl
{
    namespace reflect
    {
        class method_info;
        class parameter_info {
            friend class method_info;
            const method_info& method;
            size_t index;
            std::string name;
            any default_val;
            type ptype;
        public:
            parameter_info(const method_info& p, size_t idx, std::string pname, type t)
                : method(p), index(idx), name(pname), default_val(), ptype(t)
            {}
            parameter_info(const method_info& p, size_t idx, std::string pname, type t, any defaultval)
                : method(p), index(idx), name(pname), default_val(defaultval), ptype(t)
            {}

            const method_info& get_declaring_method() const noexcept {
                return method;
            }
            
            size_t get_index() const noexcept {
                return index;
            }

            const std::string& get_name() const noexcept {
                return name;
            }

            type get_type() const noexcept {
                return ptype;
            }

            any get_default_value() const noexcept {
                return default_val;
            }

            bool has_default_value() const noexcept {
                return default_val.valid();
            }
        };
    }
}
