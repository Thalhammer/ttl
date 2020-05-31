#pragma once
#include <memory>
#include "type.h"
#include "to_string.h"
#include <functional>
#include "cxx11_helpers.h"
#ifdef __cpp_lib_any
#include <any>
#endif
#ifdef __GLIBCXX__
#include <cxxabi.h>
#endif
#ifdef _WIN64
#ifndef _APISETLIBLOADER_
// We need GetModuleHandle to get the image base address, but do not want to include all the trash in windows.h
extern "C" __declspec(dllimport) void* __stdcall GetModuleHandleA(const char* lpModuleName);
#endif
#endif

namespace ttl
{
	namespace detail {
		template<typename T>
		class has_global_to_string
		{
			template<typename U>
			static constexpr auto check(U*)
				-> typename std::is_same<ttl::decay_t<decltype(to_string(std::declval<U>()))>, std::string>::type;

			template<typename> static constexpr std::false_type check(...);

			typedef decltype(check<T>(nullptr)) type;

		public:
			static constexpr bool value = type::value;
		};

		template<typename T>
		class has_member_to_string
		{
			template<typename U>
			static constexpr auto check(U*)
				-> typename std::is_same<ttl::decay_t<decltype(std::declval<U>().to_string())>, std::string>::type;

			template<typename> static constexpr std::false_type check(...);

			typedef decltype(check<T>(nullptr)) type;

		public:
			static constexpr bool value = type::value;
		};
	}
	class any {
		class data_base {
			ttl::type type_info;
		protected:
			explicit data_base(ttl::type info)
				:type_info(info)
			{}
		public:
			virtual ~data_base() noexcept {}
			const ttl::type& type() const noexcept { return type_info; }
			virtual void* data_ptr() noexcept = 0;
			virtual std::unique_ptr<data_base> clone() const = 0;
#ifdef __cpp_lib_any
			virtual std::any to_std_any() const = 0;
#endif
			virtual std::string to_string() const = 0;
			virtual std::function<bool(ttl::any&)> iterate() const = 0;
			//virtual std::function<void(ttl::any)> push_back() const noexcept = 0;
		};
		template<typename T>
		struct data final : data_base {
			T val;

			explicit data(T v)
				: data_base(type::create<T>()), val(v)
			{
			}

			virtual ~data() noexcept override {}
			void* data_ptr() noexcept override {
				return const_cast<void*>(static_cast<const void*>(&val));
			}
			std::unique_ptr<data_base> clone() const override {
				return ttl::make_unique<data<T>>(val);
			}
#ifdef __cpp_lib_any
			std::any to_std_any() const override {
				return val;
			}
#endif
			std::string to_string() const override {
				return to_string_impl();
			}

			std::function<bool(ttl::any&)> iterate() const override {
				return iterate_impl();
			}

			template<typename U = T>
			typename std::enable_if<detail::has_global_to_string<U>::value, std::string>::type
				to_string_impl() const {
				return ttl::to_string(val);
			}

			template<typename U = T>
			typename std::enable_if<!detail::has_global_to_string<U>::value && detail::has_member_to_string<U>::value, std::string>::type
				to_string_impl() const {
				return val.to_string();
			}

			template<typename U = T>
			typename std::enable_if<!detail::has_global_to_string<U>::value && !detail::has_member_to_string<U>::value, std::string>::type
				to_string_impl() const {
				throw std::logic_error("to_string not implemented");
			}

			template<typename U = typename std::remove_reference<T>::type>
			typename std::enable_if<traits::is_iterable<U>::value, std::function<bool(ttl::any&)>>::type
				iterate_impl() const {
				auto it = val.begin();
				auto end = val.end();
				return [it, end](ttl::any& out) mutable -> bool {
					if(it == end) return false;
					out = ttl::any::create<decltype(*it)>(*it);
					it++;
					return true;
				};
			}

			template<typename U = typename std::remove_reference<T>::type>
			typename std::enable_if<!traits::is_iterable<U>::value, std::function<bool(ttl::any&)>>::type
				iterate_impl() const {
				throw std::logic_error("value is not iterable");
			}
		};

		std::unique_ptr<data_base> val;

#ifdef __GLIBCXX__
		static void* upcast(const std::type_info& from, const std::type_info& to, void* ptr) {
			if (from == to)
				return ptr;
			if (typeid(from) == typeid(abi::__vmi_class_type_info)) {
				auto* vmi = static_cast<const abi::__vmi_class_type_info*>(&from);
				for (size_t i = 0; i < vmi->__base_count; i++) {
					const abi::__base_class_type_info* info = &(vmi->__base_info[i]);
					auto offset = info->__offset_flags >> info->__offset_shift;
					if (*info->__base_type == to)
						return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + offset);
				}
				// No type found, second pass, check children of base classes
				for (size_t i = 0; i < vmi->__base_count; i++) {
					const abi::__base_class_type_info* info = &(vmi->__base_info[i]);
					auto offset = info->__offset_flags >> info->__offset_shift;
					void* child = upcast(*info->__base_type, to, reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + offset));
					if (child)
						return child;
				}
			}
			else if (typeid(from) == typeid(abi::__si_class_type_info)) {
				// Simple case, single base class offset zero, compare and return
				auto* si = static_cast<const abi::__si_class_type_info*>(&from);
				if (si->__base_type == nullptr)
					throw std::logic_error("invalid rtti data");
				if (*si->__base_type == to)
					return ptr;
				else return upcast(*si->__base_type, to, ptr);
			}
			return nullptr;
		}
#elif defined(_WIN32)
		typedef uint32_t rva_t;

		template<typename T>
		static T* convertRVA(rva_t rva)
		{
#ifdef _WIN64
			const static uint64_t imgbase = reinterpret_cast<uint64_t>(GetModuleHandleA(nullptr));
			return reinterpret_cast<T*>(imgbase + rva);
#else
			return (T*)rva;
#endif
		}

		static void* upcast(const std::type_info& sfrom, const std::type_info& sto, void* ptr) {
			if (sfrom == sto)
				return ptr;

#pragma warning (push)
#pragma warning (disable:4200)
			struct RTTITypeDescriptor
			{
				unsigned long hash;
				void* spare;
				const char name[0];
			};

			struct RTTIBaseClassDescriptor
			{
				rva_t pTypeDescriptor; //type descriptor of the class (struct RTTITypeDescriptor*)
				uint32_t numContainedBases; //number of nested classes following in the Base Class Array
				int mdisp;  //member displacement
				int pdisp;  //vbtable displacement
				int vdisp;  //displacement inside vbtable
				uint32_t attributes;        //flags, usually 0
			};

			struct RTTIBaseClassArray {
				rva_t arrayOfBaseClassDescriptors[]; // struct RTTIBaseClassDescriptor *
			};

			struct RTTIClassHierarchyDescriptor
			{
				uint32_t signature;      //always zero?
				uint32_t attributes;     //bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
				uint32_t numBaseClasses; //number of classes in pBaseClassArray
				rva_t pBaseClassArray;
			};

			struct RTTICompleteObjectLocator
			{
				uint32_t signature; //always zero ?
				uint32_t offset;    //offset of this vtable in the complete class
				uint32_t cdOffset;  //constructor displacement offset
				rva_t pTypeDescriptor; //TypeDescriptor of the complete class (struct RTTITypeDescriptor*)
				rva_t pClassDescriptor; //describes inheritance hierarchy (struct RTTIClassHierarchyDescriptor*)
			};
#pragma warning (pop)

			void* ptrin = __RTCastToVoid(ptr);
			RTTICompleteObjectLocator *pCompleteLocator = (RTTICompleteObjectLocator *)((*((void***)ptrin))[-1]);
			auto* hierarchy = convertRVA<struct RTTIClassHierarchyDescriptor>(pCompleteLocator->pClassDescriptor);
			auto* to = (RTTITypeDescriptor*)&sto;

			auto* base_array = convertRVA<struct RTTIBaseClassArray>(hierarchy->pBaseClassArray);
			//->arrayOfBaseClassDescriptors[0]->pTypeDescriptor->name;

			for (uint32_t i = 0; i < hierarchy->numBaseClasses; i++)
			{
				auto* pBCD = convertRVA<struct RTTIBaseClassDescriptor>(base_array->arrayOfBaseClassDescriptors[i]);
				auto* type = convertRVA<struct RTTITypeDescriptor>(pBCD->pTypeDescriptor);
				if (type == to)
				{
					ptrdiff_t offset = 0;
					if (pBCD->pdisp >= 0) {
						// if base is in the virtual part of class
						offset = pBCD->pdisp;
						offset += *(__int32*)((char*)*(ptrdiff_t*)((char*)ptrin + offset) +
							pBCD->vdisp);
					}

					offset += pBCD->mdisp;
					return ((char *)ptrin) + offset;
				}
			}

			return nullptr;
		}
#else
#error Unsupported plattfrom
#endif
	public:
		// Create with explicit type
		template<typename T>
		static any create(T&& arg) {
			any res;
			res.val = ttl::make_unique<data<T>>(std::forward<T>(arg));
			return res;
		}
		// Create with implicit type
		template<typename T>
		any(T arg)
			: val(ttl::make_unique<data<T>>(std::move(arg)))
		{}

		any(const any& other)
			: val(other.val == nullptr ? nullptr : other.val->clone())
		{}

		// Create empty
		any()
		{}

		any& operator=(const any& other) {
			val = other.val == nullptr ? nullptr : other.val->clone();
			return *this;
		}

		bool valid() const noexcept {
			return !empty();
		}

		bool empty() const noexcept {
			return !val;
		}

		const std::string& pretty_name() const {
			if(empty()) throw std::logic_error("invalid any");
			return val->type().pretty_name();
		}

		bool is_type(const std::type_info& type) const {
			if(empty()) throw std::logic_error("invalid any");
			return val->type().std_type() == type;
		}

		const std::type_info& std_type() const {
			if(empty()) throw std::logic_error("invalid any");
			return val->type().std_type();
		}

		const ttl::type& type() const {
			if(empty()) throw std::logic_error("invalid any");
			return val->type();
		}

		any clone() const {
			if(empty()) throw std::logic_error("invalid any");
			any a;
			a.val = val->clone();
			return a;
		}

#ifdef __cpp_lib_any
		std::any to_std_any() const {
			if(empty()) return std::any();
			return val->to_std_any();
		}
#endif

		std::string to_string() const {
			if(empty()) throw std::logic_error("invalid any");
			return val->to_string();
		}

		std::function<bool(ttl::any&)> iterate() const {
			if(empty()) throw std::logic_error("invalid any");
			return val->iterate();
		}

#ifdef _WIN32
		const void* upcast(const std::type_info& to) const {
			if(empty()) throw std::logic_error("invalid any");
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			if (!type().is_polymorphic())
				throw std::logic_error("type needs to be polymorphic");
			return upcast(from, to, ptr);
		}

		void* upcast(const std::type_info& to) {
			if(empty()) throw std::logic_error("invalid any");
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			if (!type().is_polymorphic())
				throw std::logic_error("type needs to be polymorphic");
			return upcast(from, to, ptr);
		}
#else
		const void* upcast(const std::type_info& to) const {
			if(empty()) throw std::logic_error("invalid any");
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			if (!type().is_class())
				throw std::logic_error("type needs to be a class");
			return upcast(from, to, ptr);
		}

		void* upcast(const std::type_info& to) {
			if(empty()) throw std::logic_error("invalid any");
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			if (!type().is_class())
				throw std::logic_error("type needs to be a class");
			return upcast(from, to, ptr);
		}

#endif

		template<typename T>
		const T* upcast() const {
			return reinterpret_cast<const T*>(this->upcast(typeid(T)));
		}

		template<typename T>
		T* upcast() {
			return reinterpret_cast<T*>(this->upcast(typeid(T)));
		}

		/**
		 * Returns a pointer to the contained object.
		 * If the contained object is a reference, a pointer to the referenced object is returned.
		 * \return Pointer to object
		 */
		template<typename T>
		typename std::enable_if<std::is_reference<T>::value, typename std::remove_reference<T>::type*>::type
			get_pointer() {
			if(empty()) throw std::logic_error("invalid any");
			return reinterpret_cast<typename std::remove_reference<T>::type*>(val->data_ptr());
		}
		/**
		 * Returns a pointer to the contained object.
		 * If the contained object is a reference, a pointer to the referenced object is returned.
		 * \return Pointer to object
		 */
		template<typename T>
		typename std::enable_if<!std::is_reference<T>::value, T*>::type
			get_pointer() {
			if(empty()) throw std::logic_error("invalid any");
			return reinterpret_cast<T*>(val->data_ptr());
		}
		/**
		 * Returns a pointer to the contained object.
		 * If the contained object is a reference, a pointer to the referenced object is returned.
		 * \return Pointer to object
		 */
		template<typename T>
		typename std::enable_if<std::is_reference<T>::value, const typename std::remove_reference<T>::type*>::type
			get_pointer() const {
			if(empty()) throw std::logic_error("invalid any");
			if(typeid(typename std::remove_reference<T>::type) != val->type().std_type()) {
				throw std::logic_error(std::string("bad any cast:")
					+ ttl::type::create<typename std::remove_reference<T>::type>().pretty_name()
					+ " != " + val->type().pretty_name());
			}
			return reinterpret_cast<const typename std::remove_reference<T>::type*>(val->data_ptr());
		}
		/**
		 * Returns a pointer to the contained object.
		 * If the contained object is a reference, a pointer to the referenced object is returned.
		 * \return Pointer to object
		 */
		template<typename T>
		typename std::enable_if<!std::is_reference<T>::value, const T*>::type
			get_pointer() const {
			if(empty()) throw std::logic_error("invalid any");
			if(typeid(T) != val->type().std_type()) {
				throw std::logic_error(std::string("bad any cast:")
					+ ttl::type::create<T>().pretty_name()
					+ " != " + val->type().pretty_name());
			}
			return reinterpret_cast<const T*>(val->data_ptr());
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

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
