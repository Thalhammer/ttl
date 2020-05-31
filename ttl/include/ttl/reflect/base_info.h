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
        class base_info {
            template<typename T> friend class builder;
            type basetype;
            std::function<any(any)> convertfn;
            template<typename T> struct deduction_helper {};
            template<typename TBase, typename TDerived>
            base_info(deduction_helper<TBase> b, deduction_helper<TDerived> d) : basetype(type::create<TBase>()) {
                (void)b;
                (void)d;
                static_assert(std::is_base_of<TBase, TDerived>::value, "TBase is not a base class");
                convertfn = [](any instance) -> any {
                    TDerived* i = nullptr;
                    if(instance.is_type<TDerived>()) i = instance.get_pointer<TDerived>();
                    else if(instance.is_type<TDerived*>()) i = instance.get<TDerived*>();
                    else throw std::runtime_error("bad instance");
                    return static_cast<TBase*>(i);
                };
            }
        public:
            const type get_class() const noexcept {
                return basetype;
            }
            any convert(any instance) {
                return convertfn(instance);
            }
        };
    }
}
