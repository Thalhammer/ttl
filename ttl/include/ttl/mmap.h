#pragma once
#include <string>
#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#endif

namespace ttl {
	class mmap {
#ifdef _WIN32
		HANDLE _file;
		HANDLE _mapped;
#else
		int _file;
#endif
		uint64_t _filesize;
		uint64_t _view_size;
		const void* _view;
	public:
		mmap& operator=(const mmap& o) = delete;
		mmap(const mmap& o) = delete;

#ifdef _WIN32
		mmap()
			: _file(nullptr), _mapped(nullptr), _filesize(0), _view_size(0), _view(nullptr)
		{}
#else
		mmap()
			: _file(-1), _filesize(0), _view_size(0), _view(nullptr)
		{}
#endif

		explicit mmap(const std::string& fname)
			: mmap()
		{
			if (!open(fname))
				throw std::runtime_error("failed to open file");
		}

		~mmap() {
			close();
		}

		bool open(const std::string& fname) {
			close();
#ifdef _WIN32
			_file = ::CreateFileA(fname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (!_file)
				return false;
			// file size
			LARGE_INTEGER result;
			if (!GetFileSizeEx(_file, &result)) {
				CloseHandle(_file);
				_file = nullptr;
				return false;
			}
			_filesize = static_cast<uint64_t>(result.QuadPart);

			// convert to mapped mode
			_mapped = ::CreateFileMappingA(_file, NULL, PAGE_READONLY, 0, 0, NULL);
			if (!_mapped) {
				CloseHandle(_file);
				_file = nullptr;
				_filesize = 0;
				return false;
			}
#else
			_file = ::open(fname.c_str(), O_RDONLY | O_LARGEFILE);
			if (_file == -1)
			{
				_file = 0;
				return false;
			}

			// file size
			struct stat64 statInfo;
			if (fstat64(_file, &statInfo) < 0)
				return false;

			_filesize = statInfo.st_size;
#endif
			// initial mapping
			remap(0, 0);

			if (!_view)
				return false;

			return true;
		}

		void close() {
			// kill pointer
			if (_view)
			{
#ifdef _WIN32
				::UnmapViewOfFile(_view);
#else
				::munmap((void*)_view, _filesize);
#endif
				_view = NULL;
			}

#ifdef _WIN32
			if (_mapped)
			{
				::CloseHandle(_mapped);
				_mapped = NULL;
			}

			if (_file)
			{
				::CloseHandle(_file);
				_file = nullptr;
			}
#else
			if (_file != -1)
			{
				::close(_file);
				_file = -1;
			}
#endif

			_filesize = 0;
		}

		bool is_valid() const {
			return _view != nullptr;
		}

		bool remap(size_t offset, size_t len) {
			// don't go further than end of file
			if (offset > _filesize)
				return false;
			if (offset + len > _filesize || len == 0)
				len = size_t(_filesize - offset);

#ifdef _WIN32
			if (!_file)
				return false;

			if (_view)
			{
				::UnmapViewOfFile(_view);
				_view = nullptr;
				_view_size = 0;
			}

			DWORD offsetLow = DWORD(offset & 0xFFFFFFFF);
#ifdef _WIN64
			DWORD offsetHigh = DWORD(offset >> 32);
#else
			DWORD offsetHigh = 0;
#endif
			_view = ::MapViewOfFile(_mapped, FILE_MAP_READ, offsetHigh, offsetLow, len);
			if (_view == nullptr)
				return false;
			_view_size = len;

#else
			if (_file == -1)
				return false;

			if (_view)
			{
				::munmap((void*)_view, _view_size);
				_view = nullptr;
				_view_size = 0;
			}

			_view = ::mmap64(NULL, len, PROT_READ, MAP_SHARED, _file, offset);
			if (_view == MAP_FAILED)
			{
				_view = nullptr;
				return false;
			}
			_view_size = len;
#endif
			return true;
		}

		const uint8_t& operator[](size_t idx) const {
			return ((const uint8_t*)_view)[idx];
		}

		const uint8_t& at(size_t idx) const {
			if (!is_valid())
				throw std::runtime_error("no file mapped");
			if (idx > _view_size)
				throw std::out_of_range("invalid idx");
			return operator[](idx);
		}

		const uint8_t* data() const {
			return (const uint8_t*)_view;
		}

		size_t size() const {
			return (size_t)_view_size;
		}
	};
}

namespace thalhammer = ttl;