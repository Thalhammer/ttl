#pragma once
#include <string>
#include <vector>
#include "zip_internals.h"
#include <array>
#include "../crc.h"
#include <chrono>

namespace thalhammer {
	namespace io {
		class zip_entry {
			zip_internals::global_file_header header;
			std::string name;
			std::string comment;
			std::string extra;
			
			template<bool>
			friend class zip_stream;
			friend class zip_reader;
		public:
			zip_entry() {
				header.version_made = 20;
				header.version_needed = 20;
				header.flags = zip_internals::file_flags::data_descriptor | zip_internals::file_flags::language_encoding;
				set_last_modified(std::chrono::system_clock::now());
			}

			const zip_internals::global_file_header& get_header() const { return header; }
			const std::string& get_name() const { return name; }
			const std::string& get_comment() const { return comment; }
			const std::string& get_extra() const { return extra; }
			bool is_compressed() const { return header.method != zip_internals::compression_method::store; }
			uint16_t get_compression_method() const { return header.method; }
			bool is_directory() const { return (header.attributes_external & 0x10) != 0; }
			time_t get_last_modified() const {
				struct tm time;
				memset(&time, 0x00, sizeof(struct tm));
				time.tm_sec = (header.last_modified_time & 0x1F) * 2;
				time.tm_min = (header.last_modified_time >> 5) & 0x3F;
				time.tm_hour = (header.last_modified_time >> 11) & 0x1f;
				time.tm_year = ((header.last_modified_date >> 9) & 0x7f) + 80;
				time.tm_mon = ((header.last_modified_date >> 5) & 0x0f) - 1;
				time.tm_mday = header.last_modified_date & 0x1f;
#ifdef _WIN32
				return _mkgmtime(&time);
#else
				return timegm(&time);
#endif
			}

			void set_name(std::string name) {
				if (name.size() > std::numeric_limits<uint16_t>::max())
					throw std::invalid_argument("name to long");
				header.filename_length = (uint16_t)name.size();
				this->name = std::move(name);
			}

			void set_comment(std::string comment) {
				if (comment.size() > std::numeric_limits<uint16_t>::max())
					throw std::invalid_argument("comment to long");
				header.filecomment_length = (uint16_t)comment.size();
				this->comment = std::move(comment);
			}

			void set_extra(std::string extra) {
				if (extra.size() > std::numeric_limits<uint16_t>::max())
					throw std::invalid_argument("extra to long");

				// Validate extra format
				const uint8_t* data = reinterpret_cast<const uint8_t*>(extra.data());
				const uint8_t* const data_end = data + extra.size();
				while (data != data_end) {
					if (data_end - data < 4)
						throw std::invalid_argument("Invalid extra data format: incomplete header");
					const uint16_t* ptr = reinterpret_cast<const uint16_t*>(data);
					data += ptr[1];
					if(data > data_end)
						throw std::invalid_argument("Invalid extra data format: element exceeds field");
				}

				header.extra_length = (uint16_t)extra.size();
				this->extra = std::move(extra);
			}

			void set_last_modified(time_t t) {
				struct tm time;
#ifdef _WIN32
				gmtime_s(&time, &t);
#else
				gmtime_r(&t, &time);
#endif
				header.last_modified_time = time.tm_hour << 11 | time.tm_min << 5 | std::max(time.tm_sec / 2, 29);
				header.last_modified_date = (time.tm_year - 80) << 9 | (time.tm_mon + 1) << 5 | time.tm_mday;
			}

			void set_last_modified(const std::chrono::system_clock::time_point& tp) {
				set_last_modified(std::chrono::system_clock::to_time_t(tp));
			}

			void set_directory(bool d) {
				if (d) header.attributes_external |= 0x10;
				else header.attributes_external &= ~0x10;
			}

			void set_compressed(bool b) {
				using zip_internals::compression_method;
				set_compression_method(b ? compression_method::deflate : compression_method::store);
			}

			void set_compression_method(uint16_t v) {
				if (!zip_internals::compression_method::is_valid(v))
					throw std::invalid_argument("invalid method");
				header.method = v;
			}

			void add_extra(uint16_t id, const std::string& data) {
				if (extra.size() + 4 + data.size() > std::numeric_limits<uint16_t>::max())
					throw std::invalid_argument("extra to long");
				extra.reserve(extra.size() + 4 + data.size());
				extra.resize(extra.size() + 4);
				uint16_t* ptr = reinterpret_cast<uint16_t*>(const_cast<char*>(extra.data() + extra.size() - 4));
				ptr[0] = id;
				ptr[1] = (uint16_t)data.size();
				extra += data;
			}
		};
	}
}
