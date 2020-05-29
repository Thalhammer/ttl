#pragma once
#include <memory>
#include <type_traits>

namespace ttl
{
#ifdef __cpp_lib_make_unique
    using std::make_unique;
#else
    template <typename T, typename... Args>
    inline std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#endif
    template< class T >
    using decay_t = typename std::decay<T>::type;

    template<typename C>
    inline constexpr auto cbegin(const C& cont) noexcept(noexcept(std::begin(cont)))
        -> decltype(std::begin(cont))
        { return std::begin(cont); }
    
    template<typename C>
    inline constexpr auto cend(const C& cont) noexcept(noexcept(std::end(cont)))
        -> decltype(std::end(cont))
        { return std::end(cont); }
}