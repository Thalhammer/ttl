#pragma once
#include "constructor_info.h"
#include "method_info.h"
#include "field_info.h"

namespace ttl
{
    namespace reflect
    {
        template<typename T>
        class builder;
        class class_info {
            template<typename T> friend class builder;
            std::string name;
            std::vector<constructor_info> constructors;
            std::vector<method_info> methods;
            std::vector<field_info> fields;
            class_info() {}
        public:
            const std::string& get_name() const noexcept {
                return name;
            }
            const std::vector<constructor_info>& get_constructors() const noexcept {
                return constructors;
            }
            const std::vector<method_info>& get_methods() const noexcept {
                return methods;
            }
            const std::vector<field_info>& get_fields() const noexcept {
                return fields;
            }
        };
    }
}
