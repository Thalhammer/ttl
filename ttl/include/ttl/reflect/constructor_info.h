#pragma once
#include "../function.h"
#include "constructor_parameter_info.h"

namespace ttl
{
    namespace reflect
    {
        template<typename T>
        class builder;
        class class_info;
        class constructor_info {
            const class_info& pclass;
            std::vector<constructor_parameter_info> params;
            function fn;
            std::vector<any> attributes;
            template<typename T> friend class builder;
            constructor_info(const class_info& parent, function fnptr, const std::vector<std::string>& paramnames = {}, const std::vector<any>& defaultvals = {}, const std::vector<any>& pattributes = {})
                : pclass(parent), fn(fnptr), attributes(pattributes)
            {
                size_t idx = 0;
                for(auto& p : fn.get_parameter_types())
                    params.push_back(constructor_parameter_info(*this, idx++, "", p));

                if(paramnames.size() > params.size())
                    throw std::logic_error("Number of parameter names exceeds number of parameters");
                if(defaultvals.size() > params.size())
                    throw std::logic_error("Number of default values exceeds number of parameters");
                for(size_t i=0; i<paramnames.size(); i++) {
                    params[i].name = paramnames[i];
                }
                size_t offset = params.size() - defaultvals.size();
                for(size_t i=0; i<defaultvals.size(); i++) {
                    params[i + offset].default_val = defaultvals[i];
                }
            }
        public:
            const class_info& get_declaring_class() const noexcept {
                return pclass;
            }

            const std::vector<constructor_parameter_info>& get_parameters() const noexcept {
                return params;
            }

            const std::vector<any> get_attributes() const noexcept {
                return attributes;
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
        };
    }
}
