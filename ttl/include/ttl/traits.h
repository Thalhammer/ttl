#pragma once
#include <algorithm> 
#include <cctype>
#include <locale>
#include <limits>
#include <vector>
#include <sstream>

namespace ttl
{
	namespace traits {
		template<typename T, typename _ = void>
		struct is_iterable : std::false_type {};

		template<typename... Ts>
		struct is_iterable_helper {};

		template<typename T>
		struct is_iterable<
				T,
				std::conditional_t<
					false,
					is_iterable_helper<
						typename T::value_type,
						typename T::iterator,
						typename T::const_iterator,
						decltype(std::declval<T>().begin()),
						decltype(std::declval<T>().end()),
						decltype(std::declval<T>().cbegin()),
						decltype(std::declval<T>().cend())
						>,
					void
					>
				> : public std::true_type {};
	}
}
