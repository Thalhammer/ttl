#pragma once
#include <string>
#include <vector>
#include "zip_internals.h"
#include <array>
#include "../crc.h"
#include <chrono>

namespace ttl {
	namespace io {
		class zip_entry {
			zip_internals::global_file_header m_header;
			std::string m_name;
			std::string m_comment;
			std::string m_extra;
			
			template<bool>
			friend class zip_stream;
			friend class zip_reader;
		public:
			zip_entry() {
				m_header.version_made = 20;
				m_header.version_needed = 20;
				m_header.flags = zip_internals::file_flags::data_descriptor | zip_internals::file_flags::language_encoding;
				set_last_modified(std::chrono::system_clock::now());
			}

			const zip_internals::global_file_header& get_header() const { return m_header; }
			const std::string& get_name() const { return m_name; }
			const std::string& get_comment() const { return m_comment; }
			const std::string& get_extra() const { return m_extra; }
			bool is_compressed() const { return m_header.method != zip_internals::compression_method::store; }
			uint16_t get_compression_method() const { return m_header.method; }
			bool is_directory() const { return (m_header.attributes_external & 0x10) != 0; }
			time_t get_last_modified() const {
				struct tm time;
				memset(&time, 0x00, sizeof(struct tm));
				time.tm_sec = (m_header.last_modified_time & 0x1F) * 2;
				time.tm_min = (m_header.last_modified_time >> 5) & 0x3F;
				time.tm_hour = (m_header.last_modified_time >> 11) & 0x1f;
				time.tm_year = ((m_header.last_modified_date >> 9) & 0x7f) + 80;
				time.tm_mon = ((m_header.last_modified_date >> 5) & 0x0f) - 1;
				time.tm_mday = m_header.last_modified_date & 0x1f;
#ifdef _WIN32
				return _mkgmtime(&time);
#else
				return timegm(&time);
#endif
			}

			void set_name(std::string name) {
				if (name.size() > std::numeric_limits<uint16_t>::max())
					throw std::invalid_argument("name to long");
				m_header.filename_length = static_cast<uint16_t>(name.size());
				this->m_name = std::move(name);
			}

			void set_comment(std::string comment) {
				if (comment.size() > std::numeric_limits<uint16_t>::max())
					throw std::invalid_argument("comment to long");
				m_header.filecomment_length = static_cast<uint16_t>(comment.size());
				this->m_comment = std::move(comment);
			}

			void set_extra(std::string extra) {
				if (extra.size() > std::numeric_limits<uint16_t>::max())
					throw std::invalid_argument("extra to long");

				// Validate extra format
				auto data = reinterpret_cast<const uint8_t*>(extra.data());
				auto const data_end = data + extra.size();
				while (data != data_end) {
					if (data_end - data < 4)
						throw std::invalid_argument("Invalid extra data format: incomplete header");
					auto ptr = reinterpret_cast<const uint16_t*>(data);
					data += ptr[1];
					if(data > data_end)
						throw std::invalid_argument("Invalid extra data format: element exceeds field");
				}

				m_header.extra_length = static_cast<uint16_t>(extra.size());
				this->m_extra = std::move(extra);
			}

			void set_last_modified(time_t t) {
				struct tm time;
#ifdef _WIN32
				gmtime_s(&time, &t);
#else
				gmtime_r(&t, &time);
#endif
				m_header.last_modified_time = static_cast<uint16_t>(time.tm_hour << 11 | time.tm_min << 5 | std::max(time.tm_sec / 2, 29));
				m_header.last_modified_date = static_cast<uint16_t>((time.tm_year - 80) << 9 | (time.tm_mon + 1) << 5 | time.tm_mday);
			}

			void set_last_modified(const std::chrono::system_clock::time_point& tp) {
				set_last_modified(std::chrono::system_clock::to_time_t(tp));
			}

			void set_directory(bool d) {
				if (d) m_header.attributes_external |= uint32_t(0x10);
				else m_header.attributes_external &= ~uint32_t(0x10);
			}

			void set_compressed(bool b) {
				using zip_internals::compression_method;
				set_compression_method(b ? compression_method::deflate : compression_method::store);
			}

			void set_compression_method(uint16_t v) {
				if (!zip_internals::compression_method::is_valid(v))
					throw std::invalid_argument("invalid method");
				m_header.method = v;
			}

			void add_extra(uint16_t id, const std::string& data) {
				if (m_extra.size() + 4 + data.size() > std::numeric_limits<uint16_t>::max())
					throw std::invalid_argument("extra to long");
				m_extra.reserve(m_extra.size() + 4 + data.size());
				m_extra.resize(m_extra.size() + 4);
				uint16_t* ptr = reinterpret_cast<uint16_t*>(const_cast<char*>(m_extra.data() + m_extra.size() - 4));
				ptr[0] = id;
				ptr[1] = static_cast<uint16_t>(data.size());
				m_extra += data;
			}
		};
	}
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
