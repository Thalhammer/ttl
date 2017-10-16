#pragma once
#include <string>
#include "noncopyable.h"
#ifdef _WIN32
#include "string_util.h"
#include <Windows.h>
#else
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#endif


namespace thalhammer {
	class dynlib : public noncopyable {
	public:
#ifdef _WIN32
		typedef HMODULE native_handle_t;
#else
		typedef void* native_handle_t;
#endif

		dynlib();
		~dynlib();

		bool open(const std::string& file);
		void* get_function(const std::string& fn);
		template<typename T>
		T* get_function(const std::string& fn) {
			return (T*)get_function(fn);
		}

		const std::string& errormsg() const { return _errormsg; }
		native_handle_t native_handle() {
			return _native_handle;
		}
		bool get_symbols(std::set<std::string>& table) {
			if (!_symbols_loaded)
			{
				if (!impl_get_symbols(_symbols))
					return false;
				_symbols_loaded = true;
			}
			table = _symbols;
			return true;
		}
	private:
		bool _symbols_loaded = false;
		std::set<std::string> _symbols;
		std::string _errormsg;
		native_handle_t _native_handle;

		bool impl_get_symbols(std::set<std::string>& table);

#ifdef _WIN32
		static std::string last_error();
#endif
	};

	dynlib::dynlib() {
		_native_handle = nullptr;
	}

#ifdef _WIN32
	dynlib::~dynlib() {
		if (_native_handle != nullptr)
			FreeLibrary(_native_handle);
	}

	bool dynlib::open(const std::string& file) {
		_native_handle = LoadLibraryA(file.c_str());
		if (_native_handle == nullptr) {
			this->_errormsg = last_error();
			return false;
		}
		return true;
	}

	void* dynlib::get_function(const std::string& fn) {
		if (_native_handle == nullptr)
			return nullptr;
		void* sym = (void*)GetProcAddress(_native_handle, fn.c_str());
		if (sym == nullptr)
			this->_errormsg = last_error();
		return sym;
	}

	std::string dynlib::last_error() {
		LPTSTR lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, lpMsgBuf, (lstrlen((LPCTSTR)lpMsgBuf)), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, lpMsgBuf, (lstrlen((LPCTSTR)lpMsgBuf)), &strTo[0], size_needed, NULL, NULL);

		return string::trim_copy(strTo) + " (" + std::to_string(dw) + ")";
	}

	bool dynlib::impl_get_symbols(std::set<std::string>& table) {
		table.clear();
		// Use PE header to find export names
		// More information: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680547(v=vs.85).aspx

		const uint8_t* image = (const uint8_t*)_native_handle;
		PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)image;
		if (image[0] == 'M' && image[1] == 'Z') {
			header = (const PIMAGE_NT_HEADERS)(*((uint32_t*)(&image[0x3c])) + image);
		}
		// Get Export directory
		auto etable = (IMAGE_EXPORT_DIRECTORY*)(image + header->OptionalHeader.DataDirectory[0].VirtualAddress);
		// Get name indexes and iterate over them
		auto ptr = (uint32_t*)(&image[etable->AddressOfNames]);
		for (size_t i = 0; i < etable->NumberOfNames; i++)
			table.insert((const char*)(&image[ptr[i]]));
		return true;
	}
#else
	dynlib::~dynlib() {
		if (_native_handle != nullptr)
			dlclose(_native_handle);
	}

	bool dynlib::open(const std::string& file) {
		_native_handle = dlopen(file.c_str(), RTLD_NOW);
		if (_native_handle == nullptr) {
			this->_errormsg = dlerror();
			return false;
		}
		return true;
	}

	void* dynlib::get_function(const std::string& fn) {
		if (_native_handle == nullptr)
			return nullptr;
		void* sym = (void*)dlsym(_native_handle, fn.c_str());
		if (sym == nullptr)
			this->_errormsg = dlerror();
		return sym;
	}

	bool dynlib::impl_get_symbols(std::set<std::string>& tab) {
		tab.clear();

		auto get_ptr = [](const ElfW(Dyn)* dyn, ElfW(Word) tag)-> void* {
			for (; dyn->d_tag != DT_NULL; ++dyn) {
				if (dyn->d_tag == tag) {
					return (void*)dyn->d_un.d_ptr;
				}
			}
			return nullptr;
		};

		struct link_map* map;
		if (dlinfo(_native_handle, RTLD_DI_LINKMAP, &map) != 0) {
			this->_errormsg = dlerror();
			return false;
		}
		auto table = (ElfW(Dyn)*)map->l_ld;
		ElfW(Word)* hashtab = (ElfW(Word)*)get_ptr(table, DT_HASH);
		ElfW(Sym)* symtab = (ElfW(Sym)*)get_ptr(table, DT_SYMTAB);
		const char* strtab = (const char*)get_ptr(table, DT_STRTAB);
		if (hashtab == nullptr || symtab == nullptr || strtab == nullptr)
			return false;
		ElfW(Word) nchains = hashtab[1];
		for (ElfW(Word) i = 0; i < nchains; i++) {
			auto* entry = &symtab[i];
			auto type = entry->st_info & 0x0f;
			if ((type == STT_FUNC || type == STT_LOOS) && ((entry->st_info >> 4) == STB_GLOBAL || (entry->st_info >> 4) == STB_WEAK))
				tab.insert(&strtab[entry->st_name]);
		}
		return true;
	}
#endif
}