#pragma once
#include "../any.h"

namespace ttl
{
    namespace reflect
    {
        class constructor_info;
        class constructor_parameter_info {
            friend class constructor_info;
            const constructor_info& constructor;
            size_t index;
            std::string name;
            any default_val;
            type ptype;
        public:
            constructor_parameter_info(const constructor_info& p, size_t idx, std::string pname, type t)
                : constructor(p), index(idx), name(pname), default_val(), ptype(t)
            {}
            constructor_parameter_info(const constructor_info& p, size_t idx, std::string pname, type t, any defaultval)
                : constructor(p), index(idx), name(pname), default_val(defaultval), ptype(t)
            {}

            const constructor_info& get_declaring_constructor() const noexcept {
                return constructor;
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
