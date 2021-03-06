#pragma once
#include <memory>
#include <type_traits>
#include "traits.h"
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
#include "cxx11_helpers.h"

namespace ttl
{
	class type final {
		struct data_base {
			virtual ~data_base() noexcept {}
			virtual const std::string& pretty_name() const noexcept = 0;
				
			virtual bool is_abstract() const noexcept = 0;
			virtual bool is_arithmetic() const noexcept = 0;
			virtual bool is_array() const noexcept = 0;
			virtual bool is_class() const noexcept = 0;
			virtual bool is_compound() const noexcept = 0;
			virtual bool is_const() const noexcept = 0;
			virtual bool is_copy_assignable() const noexcept = 0;
			virtual bool is_copy_constructible() const noexcept = 0;
			virtual bool is_default_constructible() const noexcept = 0;
			virtual bool is_destructible() const noexcept = 0;
			virtual bool is_empty() const noexcept = 0;
			virtual bool is_enum() const noexcept = 0;
			virtual bool is_floating_point() const noexcept = 0;
			virtual bool is_function() const noexcept = 0;
			virtual bool is_fundamental() const noexcept = 0;
			virtual bool is_integral() const noexcept = 0;
			virtual bool is_literal_type() const noexcept = 0;
			virtual bool is_lvalue_ref() const noexcept = 0;
			virtual bool is_member_function_pointer() const noexcept = 0;
			virtual bool is_member_object_pointer() const noexcept = 0;
			virtual bool is_member_pointer() const noexcept = 0;
			virtual bool is_move_assignable() const noexcept = 0;
			virtual bool is_move_constructible() const noexcept = 0;
			virtual bool is_nothrow_assignable() const noexcept = 0;
			virtual bool is_nothrow_constructible() const noexcept = 0;
			virtual bool is_nothrow_copy_assignable() const noexcept = 0;
			virtual bool is_nothrow_copy_constructible() const noexcept = 0;
			virtual bool is_nothrow_default_constructible() const noexcept = 0;
			virtual bool is_nothrow_destructible() const noexcept = 0;
			virtual bool is_nothrow_move_assignable() const noexcept = 0;
			virtual bool is_nothrow_move_constructible() const noexcept = 0;
			virtual bool is_null_pointer() const noexcept = 0;
			virtual bool is_object() const noexcept = 0;
			virtual bool is_pod() const noexcept = 0;
			virtual bool is_pointer() const noexcept = 0;
			virtual bool is_polymorphic() const noexcept = 0;
			virtual bool is_reference() const noexcept = 0;
			virtual bool is_rvalue_ref() const noexcept = 0;
			virtual bool is_scalar() const noexcept = 0;
			virtual bool is_signed() const noexcept = 0;
			virtual bool is_standard_layout() const noexcept = 0;
			virtual bool is_trivial() const noexcept = 0;
			virtual bool is_trivially_assignable() const noexcept = 0;
			virtual bool is_trivially_constructible() const noexcept = 0;
			virtual bool is_trivially_copy_assignable() const noexcept = 0;
			virtual bool is_trivially_copy_constructible() const noexcept = 0;
			virtual bool is_trivially_default_constructible() const noexcept = 0;
			virtual bool is_trivially_destructible() const noexcept = 0;
			virtual bool is_trivially_move_assignable() const noexcept = 0;
			virtual bool is_trivially_move_constructible() const noexcept = 0;
			virtual bool is_union() const noexcept = 0;
			virtual bool is_unsigned() const noexcept = 0;
			virtual bool is_void() const noexcept = 0;
			virtual bool is_volatile() const noexcept = 0;
			virtual size_t rank() const noexcept = 0;
			virtual bool has_virtual_destructor() const noexcept = 0;
			virtual size_t extent(size_t i) const noexcept = 0;

			virtual size_t alignment() const noexcept = 0;
			virtual size_t size() const noexcept = 0;

			virtual std::shared_ptr<void> create_object() const = 0;

			virtual type base_type() const = 0;
			virtual const std::type_info& std_type() const noexcept = 0;
			virtual std::unique_ptr<data_base> clone() const = 0;

			virtual bool is_iterable() const noexcept = 0;
		};
		template<typename T>
		struct data final : data_base {
			static std::string p_name() {
				std::string res;
				#ifndef _MSC_VER
				auto* name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
				if(name != nullptr) {
					try {
						res = name;
					} catch(...) { free(name); throw; }
					free(name);
				} else {
					res = typeid(T).name();
				}
				#else
				res = typeid(T).name();
				#endif
				if (std::is_const<typename std::remove_reference<T>::type>::value)
					res += " const";
				if (std::is_volatile<typename std::remove_reference<T>::type>::value)
					res += " volatile";
				if (std::is_lvalue_reference<T>::value)
					res += "&";
				else if (std::is_rvalue_reference<T>::value)
					res += "&&";
				return res;
			}

			data()
			{}
			virtual ~data() noexcept override {}
			const std::string& pretty_name() const noexcept override {
				const static std::string n = p_name();
				return n;
			}
				
			bool is_abstract() const noexcept override {
				return std::is_abstract<T>::value;
			}

			bool is_arithmetic() const noexcept override {
				return std::is_arithmetic<T>::value;
			}

			bool is_array() const noexcept override {
				return std::is_array<T>::value;
			}

			bool is_class() const noexcept override {
				return std::is_class<T>::value;
			}

			bool is_compound() const noexcept override {
				return std::is_compound<T>::value;
			}

			bool is_const() const noexcept override {
				return std::is_const<T>::value;
			}

			bool is_copy_assignable() const noexcept override {
				return std::is_copy_assignable<T>::value;
			}

			bool is_copy_constructible() const noexcept override {
				return std::is_copy_constructible<T>::value;
			}

			bool is_default_constructible() const noexcept override {
				return std::is_default_constructible<T>::value;
			}

			bool is_destructible() const noexcept override {
				return std::is_destructible<T>::value;
			}

			bool is_empty() const noexcept override {
				return std::is_empty<T>::value;
			}

			bool is_enum() const noexcept override {
				return std::is_enum<T>::value;
			}

			bool is_floating_point() const noexcept override {
				return std::is_floating_point<T>::value;
			}

			bool is_function() const noexcept override {
				return std::is_function<T>::value;
			}

			bool is_fundamental() const noexcept override {
				return std::is_fundamental<T>::value;
			}

			bool is_integral() const noexcept override {
				return std::is_integral<T>::value;
			}

			bool is_literal_type() const noexcept override {
				return std::is_literal_type<T>::value;
			}

			bool is_lvalue_ref() const noexcept override {
				return std::is_lvalue_reference<T>::value;
			}

			bool is_member_function_pointer() const noexcept override {
				return std::is_member_function_pointer<T>::value;
			}

			bool is_member_object_pointer() const noexcept override {
				return std::is_member_object_pointer<T>::value;
			}
			
			bool is_member_pointer() const noexcept override {
				return std::is_member_pointer<T>::value;
			}

			bool is_move_assignable() const noexcept override {
				return std::is_move_assignable<T>::value;
			}

			bool is_move_constructible() const noexcept override {
				return std::is_move_constructible<T>::value;
			}
			
			bool is_nothrow_assignable() const noexcept override {
				return std::is_nothrow_assignable<T,T>::value;
			}

			bool is_nothrow_constructible() const noexcept override {
				return std::is_nothrow_constructible<T>::value;
			}
			
			bool is_nothrow_copy_assignable() const noexcept override {
				return std::is_nothrow_copy_assignable<T>::value;
			}

			bool is_nothrow_copy_constructible() const noexcept override {
				return std::is_nothrow_copy_constructible<T>::value;
			}
					
			bool is_nothrow_default_constructible() const noexcept override {
				return std::is_nothrow_default_constructible<T>::value;
			}

			bool is_nothrow_destructible() const noexcept override {
				return std::is_nothrow_destructible<T>::value;
			}
			
			bool is_nothrow_move_assignable() const noexcept override {
				return std::is_nothrow_move_assignable<T>::value;
			}

			bool is_nothrow_move_constructible() const noexcept override {
				return std::is_nothrow_move_constructible<T>::value;
			}
			
			bool is_null_pointer() const noexcept override {
				return std::is_null_pointer<T>::value;
			}
			
			bool is_object() const noexcept override {
				return std::is_object<T>::value;
			}
			
			bool is_pod() const noexcept override {
				return std::is_pod<T>::value;
			}
			
			bool is_pointer() const noexcept override {
				return std::is_pointer<T>::value;
			}

			bool is_polymorphic() const noexcept override {
				return std::is_polymorphic<T>::value;
			}

			bool is_reference() const noexcept override {
				return std::is_reference<T>::value;
			}

			bool is_rvalue_ref() const noexcept override {
				return std::is_rvalue_reference<T>::value;
			}

			bool is_scalar() const noexcept override {
				return std::is_scalar<T>::value;
			}

			bool is_signed() const noexcept override {
				return std::is_signed<T>::value;
			}

			bool is_standard_layout() const noexcept override {
				return std::is_standard_layout<T>::value;
			}

			bool is_trivial() const noexcept override {
				return std::is_trivial<T>::value;
			}

			bool is_trivially_assignable() const noexcept override {
				return std::is_trivially_assignable<T,T>::value;
			}

			bool is_trivially_constructible() const noexcept override {
				return std::is_trivially_constructible<T>::value;
			}
			
			template<typename U = T>
			static typename std::enable_if<std::is_same<U, void>::value, bool>::type
				is_trivially_copy_assignable_impl() {
				return false;
			}

			template<typename U = T>
			static typename std::enable_if<!std::is_same<U, void>::value, bool>::type
				is_trivially_copy_assignable_impl() {
				return std::is_trivially_copy_assignable<U>::value;
			}
			
			bool is_trivially_copy_assignable() const noexcept override {
				return is_trivially_copy_assignable_impl<T>();
			}

			template<typename U = T>
			static typename std::enable_if<std::is_same<U, void>::value, bool>::type
				is_trivially_copy_constructible_impl() {
				return false;
			}

			template<typename U = T>
			static typename std::enable_if<!std::is_same<U, void>::value, bool>::type
				is_trivially_copy_constructible_impl() {
				return std::is_trivially_copy_constructible<U>::value;
			}

			bool is_trivially_copy_constructible() const noexcept override {
				return is_trivially_copy_constructible_impl<T>();
			}
					
			bool is_trivially_default_constructible() const noexcept override {
				return std::is_trivially_default_constructible<T>::value;
			}

			bool is_trivially_destructible() const noexcept override {
				return std::is_trivially_destructible<T>::value;
			}

			template<typename U = T>
			static typename std::enable_if<std::is_same<U, void>::value, bool>::type
				is_trivially_move_assignable_impl() {
				return false;
			}

			template<typename U = T>
			static typename std::enable_if<!std::is_same<U, void>::value, bool>::type
				is_trivially_move_assignable_impl() {
				return std::is_trivially_move_assignable<U>::value;
			}
			
			bool is_trivially_move_assignable() const noexcept override {
				return is_trivially_move_assignable_impl<T>();
			}

			template<typename U = T>
			static typename std::enable_if<std::is_same<U, void>::value, bool>::type
				is_trivially_move_constructible_impl() {
				return false;
			}

			template<typename U = T>
			static typename std::enable_if<!std::is_same<U, void>::value, bool>::type
				is_trivially_move_constructible_impl() {
				return std::is_trivially_move_constructible<U>::value;
			}

			bool is_trivially_move_constructible() const noexcept override {
				return is_trivially_move_constructible_impl<T>();
			}

			bool is_union() const noexcept override {
				return std::is_union<T>::value;
			}

			bool is_unsigned() const noexcept override {
				return std::is_unsigned<T>::value;
			}

			bool is_void() const noexcept override {
				return std::is_void<T>::value;
			}

			bool is_volatile() const noexcept override {
				return std::is_volatile<T>::value;
			}

			size_t rank() const noexcept override {
				return std::rank<T>::value;
			}

			bool has_virtual_destructor() const noexcept override {
				return std::has_virtual_destructor<T>::value;
			}

			size_t extent(size_t i) const noexcept override {
				return extent_impl<std::rank<T>::value>(i);
			}

			template<typename U = T>
			static typename std::enable_if<std::is_same<U, void>::value, size_t>::type
				alignment_impl() {
				return 1;
			}

			template<typename U = T>
			static typename std::enable_if<!std::is_same<U, void>::value, size_t>::type
				alignment_impl() {
				return std::alignment_of<U>::value;
			}

			size_t alignment() const noexcept override {
				return alignment_impl<T>();
			}

			template<typename U = T>
			static typename std::enable_if<std::is_same<U, void>::value, size_t>::type
				size_impl() {
				return 0;
			}

			template<typename U = T>
			static typename std::enable_if<!std::is_same<U, void>::value, size_t>::type
				size_impl() {
				return sizeof(U);
			}

			size_t size() const noexcept override {
				return size_impl<T>();
			}

			std::shared_ptr<void> create_object() const override {
				return create_object_impl();
			}

			template<typename U = T>
			static typename std::enable_if<std::is_default_constructible<U>::value, std::shared_ptr<void>>::type
				create_object_impl() {
				return std::const_pointer_cast<void>(
					std::static_pointer_cast<const void>(
						std::make_shared<T>()
					)
				);
			}

			template<typename U = T>
			static typename std::enable_if<!std::is_default_constructible<U>::value, std::shared_ptr<void>>::type
				create_object_impl() {
				return nullptr;
			}

			template<size_t Rank>
			static typename std::enable_if<Rank != 0, unsigned long int>::type extent_impl(size_t i) noexcept {
				if(i == Rank)
					return std::extent<T, Rank>::value;
				return extent_impl<Rank-1>(i);
			}

			template<size_t Rank>
			static typename std::enable_if<Rank == 0, unsigned long int>::type extent_impl(size_t) noexcept {
				return std::extent<T, 0>::value;
			}

			type base_type() const override {
				return type::create<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>();
			}
			const std::type_info& std_type() const noexcept override {
				return typeid(T);
			}
			std::unique_ptr<data_base> clone() const override {
				return ttl::make_unique<data<T>>();
			}

			bool is_iterable() const noexcept override {
				return traits::is_iterable<T>::value;
			}
		};

		std::unique_ptr<data_base> val;

		// Create empty
		type()
		{}
	public:
		type(const type& o) 
			: val(o.val->clone())
		{}
		type(type&& o)
			: val(std::move(o.val))
		{}
		// Create with explicit type
		template<typename T>
		static type create() {
			type res;
			res.val = ttl::make_unique<data<T>>();
			return res;
		}

		const std::string& pretty_name() const noexcept {
			return val->pretty_name();
		}

		bool is_abstract() const noexcept {
			return val->is_abstract();
		}

		bool is_arithmetic() const noexcept {
			return val->is_arithmetic();
		}

		bool is_array() const noexcept {
			return val->is_array();
		}

		bool is_class() const noexcept {
			return val->is_class();
		}

		bool is_compound() const noexcept {
			return val->is_compound();
		}

		bool is_const() const noexcept {
			return val->is_const();
		}

		bool is_copy_assignable() const noexcept {
			return val->is_copy_assignable();
		}

		bool is_copy_constructible() const noexcept {
			return val->is_copy_constructible();
		}

		bool is_default_constructible() const noexcept {
			return val->is_default_constructible();
		}

		bool is_destructible() const noexcept {
			return val->is_destructible();
		}

		bool is_empty() const noexcept {
			return val->is_empty();
		}

		bool is_enum() const noexcept {
			return val->is_enum();
		}

		bool is_floating_point() const noexcept {
			return val->is_floating_point();
		}

		bool is_function() const noexcept {
			return val->is_function();
		}

		bool is_fundamental() const noexcept {
			return val->is_fundamental();
		}

		bool is_integral() const noexcept {
			return val->is_integral();
		}

		bool is_literal_type() const noexcept {
			return val->is_literal_type();
		}

		bool is_lvalue_ref() const noexcept {
			return val->is_lvalue_ref();
		}

		bool is_member_function_pointer() const noexcept {
			return val->is_member_function_pointer();
		}

		bool is_member_object_pointer() const noexcept {
			return val->is_member_object_pointer();
		}
		
		bool is_member_pointer() const noexcept {
			return val->is_member_pointer();
		}

		bool is_move_assignable() const noexcept {
			return val->is_move_assignable();
		}

		bool is_move_constructible() const noexcept {
			return val->is_move_constructible();
		}
		
		bool is_nothrow_assignable() const noexcept {
			return val->is_nothrow_assignable();
		}

		bool is_nothrow_constructible() const noexcept {
			return val->is_nothrow_constructible();
		}
		
		bool is_nothrow_copy_assignable() const noexcept {
			return val->is_nothrow_copy_assignable();
		}

		bool is_nothrow_copy_constructible() const noexcept {
			return val->is_nothrow_copy_constructible();
		}
				
		bool is_nothrow_default_constructible() const noexcept {
			return val->is_nothrow_default_constructible();
		}

		bool is_nothrow_destructible() const noexcept {
			return val->is_nothrow_destructible();
		}
		
		bool is_nothrow_move_assignable() const noexcept {
			return val->is_nothrow_move_assignable();
		}

		bool is_nothrow_move_constructible() const noexcept {
			return val->is_nothrow_move_constructible();
		}
		
		bool is_null_pointer() const noexcept {
			return val->is_null_pointer();
		}
		
		bool is_object() const noexcept {
			return val->is_object();
		}
		
		bool is_pod() const noexcept {
			return val->is_pod();
		}
		
		bool is_pointer() const noexcept {
			return val->is_pointer();
		}

		bool is_polymorphic() const noexcept {
			return val->is_polymorphic();
		}

		bool is_reference() const noexcept {
			return val->is_reference();
		}

		bool is_rvalue_ref() const noexcept {
			return val->is_rvalue_ref();
		}

		bool is_scalar() const noexcept {
			return val->is_scalar();
		}

		bool is_signed() const noexcept {
			return val->is_signed();
		}

		bool is_standard_layout() const noexcept {
			return val->is_standard_layout();
		}

		bool is_trivial() const noexcept {
			return val->is_trivial();
		}

		bool is_trivially_assignable() const noexcept {
			return val->is_trivially_assignable();
		}

		bool is_trivially_constructible() const noexcept {
			return val->is_trivially_constructible();
		}
		
		bool is_trivially_copy_assignable() const noexcept {
			return val->is_trivially_copy_assignable();
		}

		bool is_trivially_copy_constructible() const noexcept {
			return val->is_trivially_copy_constructible();
		}
				
		bool is_trivially_default_constructible() const noexcept {
			return val->is_trivially_default_constructible();
		}

		bool is_trivially_destructible() const noexcept {
			return val->is_trivially_destructible();
		}
		
		bool is_trivially_move_assignable() const noexcept {
			return val->is_trivially_move_assignable();
		}

		bool is_trivially_move_constructible() const noexcept {
			return val->is_trivially_move_constructible();
		}

		bool is_union() const noexcept {
			return val->is_union();
		}

		bool is_unsigned() const noexcept {
			return val->is_unsigned();
		}

		bool is_void() const noexcept {
			return val->is_void();
		}

		bool is_volatile() const noexcept {
			return val->is_volatile();
		}

		size_t rank() const noexcept {
			return val->rank();
		}

		bool has_virtual_destructor() const noexcept {
			return val->has_virtual_destructor();
		}

		size_t extent(size_t i) const noexcept {
			return val->extent(i);
		}

		size_t alignment() const noexcept {
			return val->alignment();
		}

		size_t size() const noexcept {
			return val->size();
		}

		std::shared_ptr<void> create_object() const {
			return val->create_object();
		}

		type base_type() const {
			return val->base_type();
		}

		const std::type_info& std_type() const noexcept {
			return val->std_type();
		}
		
		bool is_iterable() const noexcept {
			return val->is_iterable();
		}
	};

	inline bool operator==(const type& a, const type& b) noexcept {
		return a.std_type() == b.std_type();
	}

	inline bool operator!=(const type& a, const type& b) noexcept {
		return a.std_type() != b.std_type();
	}
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
