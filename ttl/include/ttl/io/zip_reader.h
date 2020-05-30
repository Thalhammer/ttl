#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <istream>
#include "zip_internals.h"
#include "zip_entry.h"
#include "inflater.h"
#include "../cxx11_helpers.h"
#include "../crc.h"

namespace ttl {
	namespace io {
		class zip_reader {
		public:
			class zip_reader_istream;
			class reader_entry : private zip_entry {
				mutable std::mutex mtx;
				const uint8_t* raw_datastart;
				// Only filled if file was compressed
				std::vector<uint8_t> uncompressed;

				friend class zip_reader;
			public:
				reader_entry()
					: raw_datastart(nullptr)
				{}
				reader_entry(const reader_entry& other)
					: zip_entry(other)
				{
					std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
					std::unique_lock<std::mutex> lck2(other.mtx, std::defer_lock);
					std::lock(lck, lck2);
					raw_datastart = other.raw_datastart;
					uncompressed = other.uncompressed;
				}

				using zip_entry::get_header;
				using zip_entry::get_name;
				using zip_entry::get_comment;
				using zip_entry::get_extra;
				using zip_entry::is_compressed;
				using zip_entry::get_compression_method;
				using zip_entry::is_directory;
				using zip_entry::get_last_modified;

				void uncompress() {
					if (zip_entry::is_compressed() && uncompressed.size() != header.uncompressed_size) {
						if (zip_entry::get_compression_method() != zip_internals::compression_method::deflate)
							throw std::runtime_error("only deflate and store is supported");
						std::lock_guard<std::mutex> lck(mtx);
						inflater inf(15, inflater::wrapper::none);
						uncompressed = inflater::uncompress(raw_datastart, header.compressed_size, inf);
						if (uncompressed.size() != header.uncompressed_size)
							throw std::runtime_error("size missmatch");
						if (header.crc32 != CRC_32::get_crc(uncompressed))
							throw std::runtime_error("crc missmatch");
					}
				}

				inline std::unique_ptr<zip_reader::zip_reader_istream> open_stream(bool cache = true);
			};
		private:
			class zip_reader_istreambuf;

			const uint8_t* const data;
			const uint8_t* const dataend;

			const uint8_t* zip_start;
			const zip_internals::end_record* zip_endrecord;

			std::vector<reader_entry> files;
			std::unordered_multimap<std::string, size_t> filename_lookup;

			inline const zip_internals::end_record* find_endrecord() const;
			inline std::vector<reader_entry> read_centraldirectory() const;

			bool check_pointer(const void* ptr_start, size_t len) const {
				if (ptr_start < zip_start || ptr_start > dataend)
					return false;
				auto offset = dataend - reinterpret_cast<const uint8_t*>(ptr_start);
				if (offset < 0 || (size_t)offset < len)
					return false;
				return true;
			}
		public:
			zip_reader(const uint8_t* dptr, size_t dlen)
				: data(dptr), dataend(dptr + dlen)
			{
				// Find end of zip record
				zip_endrecord = find_endrecord();
				if (zip_endrecord == nullptr)
					throw std::runtime_error("could not find endrecord");
				if (zip_endrecord->disk_number != 0 || zip_endrecord->central_directory_disk_number != 0 || zip_endrecord->num_entries != zip_endrecord->num_entries_this_disk)
					throw std::runtime_error("Multidisk zip files are not supported");

				// Set zip_start to point to the real start of this zip file (might differ from data if there is data in front of zip file)
				zip_start = dataend - sizeof(zip_internals::end_record) - zip_endrecord->central_directory_size - zip_endrecord->central_directory_offset;
				if (!check_pointer(zip_start, 0))
					throw std::runtime_error("invalid zip file");

				files = read_centraldirectory();
				for (size_t i = 0; i < files.size(); i++) {
					if (!files[i].is_compressed()) {
						if (files[i].header.crc32 != CRC_32::get_crc(files[i].raw_datastart, files[i].header.uncompressed_size))
							throw std::runtime_error("crc missmatch");
					}
					filename_lookup.insert({ files[i].get_name(), i });
				}
			}

			void uncompress() {
				for (auto& e : files) {
					e.uncompress();
				}
			}

			bool has_file(const std::string& path) const {
				return filename_lookup.find(path) != filename_lookup.end();
			}

			std::vector<std::string> get_files() const {
				std::vector<std::string> res;
				for (auto& e : files)
					res.push_back(e.get_name());
				return res;
			}

			size_t get_num_entries() const { return files.size(); }
			reader_entry& get_entry(size_t idx) {
				if (idx >= files.size())
					throw std::out_of_range("invalid idx");
				return files.at(idx);
			}

			size_t find_by_path(const std::string& path) {
				auto it = filename_lookup.find(path);
				if (it != filename_lookup.end())
					return it->second;
				throw std::runtime_error("path not found");
			}

			std::vector<size_t> find_all_by_path(const std::string& path) const {
				auto it = filename_lookup.find(path);
				std::vector<size_t> res;
				for (; it != filename_lookup.end(); it++)
					res.push_back(it->second);
				return res;
			}
		};

		class zip_reader::zip_reader_istreambuf : public std::streambuf {
			std::array<char, 4096> buf;
			reader_entry& entry;
			size_t offset;
			bool cache;

			inflater decompressor;
		public:
			zip_reader_istreambuf(reader_entry& en, bool c)
				: entry(en), offset(0), cache(c), decompressor(15, inflater::wrapper::none)
			{
				// Force call to underflow
				setg(buf.data(), buf.data(), buf.data());
				decompressor.set_input(entry.raw_datastart, entry.header.compressed_size);
			}

			~zip_reader_istreambuf() {
			}

		private:
			std::streambuf::int_type underflow() override {
				if (gptr() < egptr())
					return traits_type::to_int_type(*gptr());

				assert(gptr() == egptr());
				std::lock_guard<std::mutex> lck(entry.mtx);
				if (entry.is_compressed() && entry.uncompressed.size() != entry.header.uncompressed_size) {
					// Decompress into buffer
					decompressor.set_output(reinterpret_cast<uint8_t*>(buf.data()), buf.size());
					size_t read, written;
					decompressor.uncompress(read, written);

					if (written == 0 && decompressor.finished())
						return traits_type::eof();

					if (cache) { // Update cache
						entry.uncompressed.resize(std::max(entry.uncompressed.size(), offset + written));
						memcpy(entry.uncompressed.data() + offset, buf.data(), written);
					}
					offset += written;
					setg(buf.data(), buf.data(), buf.data() + written);
				}
				else {
					// Simply copy uncompressed data to buffer
					const uint8_t* data = nullptr;
					size_t size = 0;
					if (entry.is_compressed()) {
						data = entry.uncompressed.data();
						size = std::min<size_t>(buf.size(), entry.uncompressed.size() - offset);
					}
					else {
						data = entry.raw_datastart;
						size = std::min<size_t>(buf.size(), entry.header.uncompressed_size - offset);
					}
					if (size == 0 || data == nullptr)
						return traits_type::eof();
					memcpy(buf.data(), data + offset, size);
					setg(buf.data(), buf.data(), buf.data() + size);
					offset += size;
				}

				return traits_type::to_int_type(*gptr());
			}
		};

		class zip_reader::zip_reader_istream : private zip_reader_istreambuf, public std::istream {
		public:
			zip_reader_istream(reader_entry& entry, bool cache)
				: zip_reader_istreambuf(entry, cache), std::istream(this)
			{
			}
		};

		const zip_internals::end_record* zip_reader::find_endrecord() const {
			auto end = std::max(data, dataend - sizeof(zip_internals::end_record) - 65536);
			for (const uint8_t* ptr = dataend - sizeof(zip_internals::end_record); ptr >= end; ptr--) {
				auto zer = reinterpret_cast<const zip_internals::end_record*>(ptr);
				if (zer->signature == 0x06054b50) {
					return zer;
				}
			}
			return nullptr;
		}

		std::vector<zip_reader::reader_entry> zip_reader::read_centraldirectory() const {
			std::vector<reader_entry> result;

			const uint8_t* ptr = zip_start + zip_endrecord->central_directory_offset;
			for (size_t i = 0; i < zip_endrecord->num_entries; i++) {
				if (!check_pointer(ptr, sizeof(zip_internals::global_file_header)))
					throw std::runtime_error("invalid zip file");

				auto entry = reinterpret_cast<const zip_internals::global_file_header*>(ptr);
				auto localentry = reinterpret_cast<const zip_internals::local_file_header*>(zip_start + entry->relative_header_offset);
				auto filename = reinterpret_cast<const char*>(ptr + sizeof(zip_internals::global_file_header));
				auto extra = filename + entry->filename_length;
				auto comment = extra + entry->extra_length;
				auto dataptr = zip_start + entry->relative_header_offset + sizeof(zip_internals::local_file_header) + localentry->filename_length + localentry->extra_length;
				ptr = reinterpret_cast<const uint8_t*>(comment + entry->filecomment_length);

				if (!check_pointer(filename, entry->filename_length)
					|| !check_pointer(extra, entry->extra_length)
					|| !check_pointer(comment, entry->filecomment_length)
					|| !check_pointer(dataptr, entry->compressed_size))
					throw std::runtime_error("invalid zip file");

				reader_entry e;
				e.header = *entry;
				e.name = std::string(filename, entry->filename_length);
				e.extra = std::string(extra, entry->extra_length);
				e.comment = std::string(comment, entry->filecomment_length);
				e.raw_datastart = dataptr;
				result.push_back(std::move(e));
			}

			return result;
		}

		std::unique_ptr<zip_reader::zip_reader_istream> zip_reader::reader_entry::open_stream(bool cache) {
			return ttl::make_unique<zip_reader::zip_reader_istream>(*this, cache);
		}
	}
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
