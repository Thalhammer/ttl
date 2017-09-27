#pragma once
#include <string>
#include "noncopyable.h"
#ifdef _WIN32
#include "string_util.h"
#include <Windows.h>
#else
#include <dlfcn.h>
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
	private:
		std::string _errormsg;
		native_handle_t _native_handle;

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
#endif
}
