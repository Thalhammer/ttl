#pragma once
#include <memory>
#include "type.h"
#include "to_string.h"
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
				-> typename std::is_same<std::decay_t<decltype(to_string(std::declval<U>()))>, std::string>::type;

			template<typename> static constexpr std::false_type check(...);

			typedef decltype(check<T>(0)) type;

		public:
			static constexpr bool value = type::value;
		};

		template<typename T>
		class has_member_to_string
		{
			template<typename U>
			static constexpr auto check(U*)
				-> typename std::is_same<std::decay_t<decltype(std::declval<U>().to_string())>, std::string>::type;

			template<typename> static constexpr std::false_type check(...);

			typedef decltype(check<T>(0)) type;

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
		};
		template<typename T>
		struct data final : data_base {
			T val;

			explicit data(T v)
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
			std::string to_string() const {
				return to_string_impl();
			}

			template<typename U = T>
			typename std::enable_if<detail::has_global_to_string<U>::value, std::string>::type
				to_string_impl() const {
				return std::to_string(val);
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
		};

		std::unique_ptr<data_base> val;

		// Create empty
		any()
		{}

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

		const ttl::type& type() const noexcept {
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

		std::string to_string() const {
			return val->to_string();
		}

#ifdef _WIN32
		const void* upcast(const std::type_info& to) const {
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			if (!type().is_polymorphic())
				throw std::logic_error("type needs to be polymorphic");
			return upcast(from, to, ptr);
		}

		void* upcast(const std::type_info& to) {
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			if (!type().is_polymorphic())
				throw std::logic_error("type needs to be polymorphic");
			return upcast(from, to, ptr);
		}
#else
		const void* upcast(const std::type_info& to) const {
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			if (!type().is_class())
				throw std::logic_error("type needs to be a class");
			return upcast(from, to, ptr);
		}

		void* upcast(const std::type_info& to) {
			auto& from = this->std_type();
			auto* ptr = this->val->data_ptr();
			if (!type().is_class())
				throw std::logic_error("type needs to be a class");
			return upcast(from, to, ptr);
		}

#endif

		template<typename T>
		const T* upcast() const {
			return (const T*)this->upcast(typeid(T));
		}

		template<typename T>
		T* upcast() {
			return (T*)this->upcast(typeid(T));
		}

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

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
