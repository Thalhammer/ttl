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
		
		#ifdef __GLIBCXX__
		static void* upcast(const std::type_info& from, const std::type_info& to, void* ptr) {
			if(from == to)
				return ptr;
			if(typeid(from) == typeid(abi::__vmi_class_type_info)) {
				auto* vmi = static_cast<const abi::__vmi_class_type_info*>(&from);
				for(size_t i = 0; i<vmi->__base_count; i++) {
					const abi::__base_class_type_info* info = &(vmi->__base_info[i]);
					auto offset = info->__offset_flags >> info->__offset_shift;
					if(*info->__base_type == to)
						return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + offset);
				}
				// No type found, second pass, check children of base classes
				for(size_t i = 0; i<vmi->__base_count; i++) {
					const abi::__base_class_type_info* info = &(vmi->__base_info[i]);
					auto offset = info->__offset_flags >> info->__offset_shift;
					void* child = upcast(*info->__base_type, to, reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + offset));
					if(child)
						return child;
				}
			} else if(typeid(from) == typeid(abi::__si_class_type_info)) {
				// Simple case, single base class offset zero, compare and return
				auto* si = static_cast<const abi::__si_class_type_info*>(&from);
				if(si->__base_type == nullptr)
					throw std::logic_error("invalid rtti data");
				if(*si->__base_type == to)
					return ptr;
				else return upcast(*si->__base_type, to, ptr);
			}
			return nullptr;
		}
		#endif
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

		#ifdef __GLIBCXX__
		const void* upcast(const std::type_info& to) const noexcept {
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			return upcast(from, to, ptr);
		}

		template<typename T>
		const T* upcast() const noexcept {
			return (const T*)this->upcast(typeid(T));
		}

		void* upcast(const std::type_info& to) noexcept {
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			return upcast(from, to, ptr);
		}

		template<typename T>
		T* upcast() noexcept {
			return (T*)this->upcast(typeid(T));
		}
		#endif

		template<typename T>
		T* get_pointer() noexcept {
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