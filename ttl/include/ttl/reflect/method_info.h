#pragma once
#include "../function.h"
#include "parameter_info.h"

namespace ttl
{
    namespace reflect
    {
        template<typename T>
        class builder;
        class class_info;
        class method_info {
            // Might be null if global method
            const class_info* pclass;
            std::string name;
            std::vector<parameter_info> params;
            function fn;
            template<typename T> friend class builder;
            method_info(const class_info* parent, function fnptr)
                : pclass(parent), fn(fnptr)
            {
                size_t idx = 0;
                for(auto& p : fn.get_parameter_types())
                    params.push_back(parameter_info(*this, idx++, "", p));
            }
        public:
            const class_info* get_declaring_class() const noexcept {
                return pclass;
            }

            const std::string& get_name() const noexcept {
                return name;
            }

            const std::vector<parameter_info>& get_parameters() const noexcept {
                return params;
            }

            bool is_static() const noexcept {
                return !fn.requires_instance();
            }

            optional<any> invoke(std::vector<any> params) const {
                // Fill missing arguments with defaults
                for(size_t i=params.size(); i < this->params.size(); i++) {
                    if(!this->params[i].has_default_value())
                        throw std::logic_error("missing argument for parameter and no default value found");
                    params.push_back(this->params[i].get_default_value());
                }
                return fn.invoke_dynamic(params);
            }

            optional<any> invoke(any& instance, std::vector<any> params) const {
                for(size_t i=params.size(); i < this->params.size(); i++) {
                    if(!this->params[i].has_default_value())
                        throw std::logic_error("missing argument for parameter and no default value found");
                    params.push_back(this->params[i].get_default_value());
                }
                return fn.invoke_dynamic(instance, params);
            }
        };
    }
}
