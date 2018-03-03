#pragma once
#include <string>
#include <vector>
#include "zip_internals.h"
#include "../crc.h"
#include "zip_entry.h"
#include "inflater.h"
#include <mutex>

namespace thalhammer {
	namespace io {
		class zip_reader {
		public:
			class reader_entry : public zip_entry {
				mutable std::mutex mtx;
				const uint8_t* raw_datastart;
				// Only filled if file was compressed
				std::vector<uint8_t> uncompressed;

				friend class zip_reader;
			public:
				reader_entry() {}
				reader_entry(const reader_entry& other)
					: zip_entry(other)
				{
					std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
					std::unique_lock<std::mutex> lck2(other.mtx, std::defer_lock);
					std::lock(lck, lck2);
					raw_datastart = other.raw_datastart;
					uncompressed = other.uncompressed;
				}

				void uncompress() {
					if (zip_entry::is_compressed()) {
						if (zip_entry::get_compression_method() != zip_internals::compression_method::deflate)
							throw std::runtime_error("only deflate and store is supported");

					}
				}
			};
		private:

			const uint8_t* const data;
			const uint8_t* const dataend;

			const uint8_t* zip_start;
			const zip_internals::end_record* zip_endrecord;

			std::vector<reader_entry> files;

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
				auto filename = reinterpret_cast<const char*>(ptr + sizeof(zip_internals::global_file_header));
				auto extra = filename + entry->filename_length;
				auto comment = extra + entry->extra_length;
				auto dataptr = zip_start + entry->relative_header_offset + sizeof(zip_internals::local_file_header);
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

	}
}
