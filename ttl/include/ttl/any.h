#pragma once
#include <memory>
#include "type.h"
#ifdef __cpp_lib_any
#include <any>
#endif
#ifdef __GLIBCXX__
#include <cxxabi.h>
#endif

namespace thalhammer
{
	class any {
		class data_base {
			thalhammer::type type_info;
		protected:
			data_base(thalhammer::type info)
				:type_info(info)
			{}
		public:
			virtual ~data_base() noexcept {}
			const thalhammer::type& type() const noexcept { return type_info; }
			virtual void* data_ptr() noexcept = 0;
			virtual std::unique_ptr<data_base> clone() const = 0;
			#ifdef __cpp_lib_any
			virtual std::any to_std_any() const = 0;
			#endif
		};
		template<typename T>
		struct data final : data_base {
			T val;

			data(T v)
				: data_base(type::create<T>()), val(v)
			{
			}

			virtual ~data() noexcept {}
			void* data_ptr() noexcept {
				return &val;
			}
			std::unique_ptr<data_base> clone() const {
				return std::make_unique<data<T>>(val);
			}
			#ifdef __cpp_lib_any
			std::any to_std_any() const {
				return val;
			}
			#endif
		};

		std::unique_ptr<data_base> val;

		// Create empty
		any()
		{}
	public:
		// Create with explicit type
		template<typename T>
		static any create(T&& arg) {
			any res;
			res.val = std::make_unique<data<T>>(std::forward<T>(arg));
			return res;
		}
		// Create with implicit type
		template<typename T>
		any(T arg)
			: val(std::make_unique<data<T>>(std::move(arg)))
		{
		}

		const std::string& pretty_name() const noexcept {
			return val->type().pretty_name();
		}

		bool is_type(const std::type_info& type) const noexcept {
			return val->type().std_type() == type;
		}

		const std::type_info& std_type() const noexcept {
			return val->type().std_type();
		}

		const thalhammer::type& type() const noexcept {
			return val->type();
		}

		any clone() const {
			any a;
			a.val = val->clone();
			return a;
		}

		#ifdef __cpp_lib_any
		std::any to_std_any() const {
			return val->to_std_any();
		}
		#endif

		template<typename T>
		T* get_pointer() {
			return (T*)val->data_ptr();
		}
		template<typename T>
		const T* get_pointer() const {
			return (const T*)val->data_ptr();
		}

		template<typename T>
		T& get_reference() {
			return *get_pointer<T>();
		}
		template<typename T>
		const T& get_reference() const {
			return *get_pointer<T>();
		}
		
		template<typename T>
		T get() const {
			return *get_pointer<T>();
		}

		template<typename T>
		bool is_type() const {
			return is_type(typeid(T));
		}
	};
}