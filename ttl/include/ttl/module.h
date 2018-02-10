#pragma once
#include <string>
#ifdef _WIN32
#include <Windows.h>
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)
#else
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <linux/limits.h>
#endif

namespace thalhammer {
	class module {
	public:
		const std::string& get_filename() const { return filename; }

#ifdef _WIN32
		typedef HMODULE native_handle_t;

		static module from_handle(native_handle_t hdl) {
			module res;
			res.filename.resize(MAX_PATH);
			if (GetModuleFileNameA(hdl, (LPSTR)res.filename.data(), (DWORD)res.filename.size()) == 0)
				throw std::runtime_error("Failed to get filename");
			res.filename.resize(strlen(res.filename.c_str()));
			return res;
		}

		static module from_address(void* address) {
			HMODULE cmodule;
			if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)address, &cmodule) == 0)
				throw std::runtime_error("Failed to get Module");

			module m;
			try {
				m = module::from_handle(cmodule);
			}
			catch (const std::exception&) {
				FreeModule(cmodule);
				throw;
			}
			FreeModule(cmodule);
			return m;
		}

		// We force this method to get inlined, thus _ReturnAddress gets the callers caller.
		__forceinline static module calling_module() {
			return from_address(_ReturnAddress());
		}

		// We force this method to never get inlined, thus _ReturnAddress gets the caller.
		__declspec(noinline) static module current_module() {
			return from_address(_ReturnAddress());
		}

		static module entry_module() {
			return module::from_handle(GetModuleHandle(NULL));
		}
#else
		typedef void* native_handle_t;

		static module from_handle(native_handle_t hdl) {
			throw std::logic_error("Not implemented");
		}

		static module from_address(void* address) {
			module res;
			Dl_info info;
			if (dladdr((void*)address, &info) == 0)
				throw std::runtime_error("Failed to get module");
			res.filename.resize(PATH_MAX);
			if (realpath(info.dli_fname, (char*)res.filename.data()) == NULL)
				throw std::runtime_error("Failed to get module path");
			res.filename.resize(strlen(res.filename.c_str()));
			return res;
		}

		// We force this method to get inlined, thus _ReturnAddress gets the callers caller.
		__attribute__((always_inline)) static module calling_module() {
			return from_address(__builtin_extract_return_addr(__builtin_return_address(0)));
		}

		// We force this method to never get inlined, thus _ReturnAddress gets the caller.
		__attribute__((noinline)) static module current_module() {
			return from_address(__builtin_extract_return_addr(__builtin_return_address(0)));
		}

		static module entry_module() {
			module m;
			m.filename.resize(PATH_MAX);
			if (readlink("/proc/self/exe", (char*)m.filename.data(), m.filename.size()) == -1)
				throw std::runtime_error("Failed to get module");
			m.filename.resize(strlen(m.filename.c_str()));
			return m;
		}

#endif
	private:
		std::string filename;
	};
}
