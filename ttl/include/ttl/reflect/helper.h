#pragma once

namespace ttl
{
    namespace reflect
    {
        template<typename Signature>
        auto select_overload(Signature* func) -> decltype(func)
        {
            return func;
        }

        template<typename Signature, typename ClassType>
        auto select_overload(Signature (ClassType::*func)) -> decltype(func)
        {
            return func;
        }

        template<typename ClassType, typename ReturnType, typename... Args>
        auto select_const(ReturnType (ClassType::*func)(Args...) const) -> decltype(func)
        {
            return func;
        }

        template<typename ClassType, typename ReturnType, typename... Args>
        auto select_non_const(ReturnType(ClassType::*func)(Args...)) -> decltype(func)
        {
            return func;
        }
    }
}