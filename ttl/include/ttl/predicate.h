#pragma once
#include <type_traits>
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <list>
#include <forward_list>
#include <stack>
#include <algorithm>

namespace ttl {
	namespace predicate {
		namespace detail {
			template<typename T1>
			struct cmp_eq {
				T1 value;
				template<typename T2>
				bool operator()(const T2& other) const {
					return value == other;
				}
			};

			template<typename T1>
			struct cmp_ne {
				T1 value;
				template<typename T2>
				bool operator()(const T2& other) const {
					return value != other;
				}
			};

			template<typename T1>
			struct cmp_gt {
				T1 value;
				template<typename T2>
				bool operator()(const T2& other) const {
					return other > value;
				}
			};

			template<typename T1>
			struct cmp_ge {
				T1 value;
				template<typename T2>
				bool operator()(const T2& other) const {
					return other >= value;
				}
			};

			template<typename T1>
			struct cmp_lt {
				T1 value;
				template<typename T2>
				bool operator()(const T2& other) const {
					return other < value;
				}
			};

			template<typename T1>
			struct cmp_le {
				T1 value;
				template<typename T2>
				bool operator()(const T2& other) const {
					return other <= value;
				}
			};

			template<typename CMP1, typename CMP2>
			struct cmp_or {
				CMP1 cmp1;
				CMP2 cmp2;
				template<typename T>
				bool operator()(const T& other) const {
					return cmp1(other) || cmp2(other);
				}
			};

			template<typename CMP1, typename CMP2>
			struct cmp_and {
				CMP1 cmp1;
				CMP2 cmp2;
				template<typename T>
				bool operator()(const T& other) const {
					return cmp1(other) && cmp2(other);
				}
			};

			template<typename CMP1, typename CMP2>
			struct cmp_xor {
				CMP1 cmp1;
				CMP2 cmp2;
				template<typename T>
				bool operator()(const T& other) const {
					return !(cmp1(other) == cmp2(other));
				}
			};

			template<typename CMP>
			struct cmp_not {
				CMP cmp;
				template<typename T>
				bool operator()(const T& other) const {
					return !cmp(other);
				}
			};
		}

		template<typename T>
		inline auto equals(T val) -> detail::cmp_eq<T> {
			return detail::cmp_eq<T>{ val };
		}
		
		template<typename T>
		inline auto not_equals(T val) -> detail::cmp_ne<T> {
			return detail::cmp_ne<T>{ val };
		}

		template<typename T>
		inline auto greater(T val) -> detail::cmp_gt<T> {
			return detail::cmp_gt<T>{ val };
		}

		template<typename T>
		inline auto greater_equals(T val) -> detail::cmp_ge<T> {
			return detail::cmp_ge<T>{ val };
		}

		template<typename T>
		inline auto less(T val) -> detail::cmp_lt<T> {
			return detail::cmp_lt<T>{ val };
		}

		template<typename T>
		inline auto less_equals(T val) -> detail::cmp_le<T> {
			return detail::cmp_le<T>{ val };
		}

		template<typename T>
		inline auto eq(T val) -> detail::cmp_eq<T> {
			return detail::cmp_eq<T>{ val };
		}

		template<typename T>
		inline auto ne(T val) -> detail::cmp_ne<T> {
			return detail::cmp_ne<T>{ val };
		}

		template<typename T>
		inline auto gt(T val) -> detail::cmp_gt<T> {
			return detail::cmp_gt<T>{ val };
		}

		template<typename T>
		inline auto ge(T val) -> detail::cmp_ge<T> {
			return detail::cmp_ge<T>{ val };
		}

		template<typename T>
		inline auto lt(T val) -> detail::cmp_lt<T> {
			return detail::cmp_lt<T>{ val };
		}

		template<typename T>
		inline auto le(T val) -> detail::cmp_le<T> {
			return detail::cmp_le<T>{ val };
		}

		template<typename CMP1, typename CMP2>
		inline auto either(CMP1 cmp1, CMP2 cmp2) -> detail::cmp_or<CMP1, CMP2> {
			return detail::cmp_or<CMP1, CMP2>{ cmp1, cmp2 };
		}

		template<typename CMP1, typename CMP2>
		inline auto both(CMP1 cmp1, CMP2 cmp2) -> detail::cmp_and<CMP1, CMP2> {
			return detail::cmp_and<CMP1, CMP2>{ cmp1, cmp2 };
		}

		template<typename CMP1, typename CMP2>
		inline auto oneof(CMP1 cmp1, CMP2 cmp2) -> detail::cmp_xor<CMP1, CMP2> {
			return detail::cmp_xor<CMP1, CMP2>{ cmp1, cmp2 };
		}

		template<typename CMP>
		inline auto invert(CMP cmp) -> detail::cmp_not<CMP> {
			return detail::cmp_not<CMP>{ cmp };
		}
	}
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
