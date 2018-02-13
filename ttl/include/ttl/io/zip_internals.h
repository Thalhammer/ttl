#pragma once
#include <string>

namespace thalhammer {
	namespace io {
		namespace zip_internals {
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

			struct end_record {
				end_record() {
					memset(this, 0x00, sizeof(struct end_record));
					signature = 0x06054b50;
				}

				uint32_t signature;
				uint16_t disk_number;
				uint16_t central_directory_disk_number;
				uint16_t num_entries_this_disk;
				uint16_t num_entries;
				uint32_t central_directory_size;
				uint32_t central_directory_offset;
				uint16_t zip_comment_length;
				// Zipcomment
#ifndef _MSC_VER
			} __attribute__((__packed__));
#else
			};
#endif

			struct file_flags {
				constexpr static uint16_t encrypted = 0x0001;
				constexpr static uint16_t compression_option = 0x0006;
				constexpr static uint16_t data_descriptor = 0x0008;
				constexpr static uint16_t enhanced_deflation = 0x0010;
				constexpr static uint16_t compressed_patched_data = 0x0020;
				constexpr static uint16_t strong_encryption = 0x0040;
				constexpr static uint16_t language_encoding = 0x0800;
				constexpr static uint16_t mask_header_values = 0x2000;
			};

			struct compression_method {
				constexpr static uint16_t store = 0x0000;
				constexpr static uint16_t shrunk = 0x0001;
				constexpr static uint16_t reduced_1 = 0x0002;
				constexpr static uint16_t reduced_2 = 0x0003;
				constexpr static uint16_t reduced_3 = 0x0004;
				constexpr static uint16_t reduced_4 = 0x0005;
				constexpr static uint16_t implode = 0x0006;
				constexpr static uint16_t tokenize = 0x0007;
				constexpr static uint16_t deflate = 0x0008;
				constexpr static uint16_t deflate64 = 0x0009;
				constexpr static uint16_t pkware_implode = 0x000A;
				constexpr static uint16_t bzip2 = 0x000C;
				constexpr static uint16_t lzma = 0x000E;
				constexpr static uint16_t ibm_terse = 0x0012;
				constexpr static uint16_t pfs = 0x0013;
				constexpr static uint16_t wavpack = 0x0061;
				constexpr static uint16_t ppmd_1 = 0x0062;

				static bool is_valid(uint16_t v) {
					switch (v) {
					case store:
					case shrunk:
					case reduced_1:
					case reduced_2:
					case reduced_3:
					case reduced_4:
					case implode:
					case tokenize:
					case deflate:
					case deflate64:
					case pkware_implode:
					case bzip2:
					case lzma:
					case ibm_terse:
					case pfs:
					case wavpack:
					case ppmd_1:
						return true;
					default: return false;
					}
				}
			};

			struct global_file_header {
				global_file_header() {
					memset(this, 0x00, sizeof(struct global_file_header));
					signature = 0x02014B50;
				}
				uint32_t signature;
				uint16_t version_made;
				uint16_t version_needed;
				uint16_t flags;
				uint16_t method;
				uint16_t last_modified_time;
				uint16_t last_modified_date;
				uint32_t crc32;
				uint32_t compressed_size;
				uint32_t uncompressed_size;
				uint16_t filename_length;
				uint16_t extra_length;
				uint16_t filecomment_length;
				uint16_t disk_number_start;
				uint16_t attributes_internal;
				uint32_t attributes_external;
				uint32_t relative_header_offset;
				// Filename
				// Extra
				// Comment
#ifndef _MSC_VER
			} __attribute__((__packed__));
#else
			};
#endif

			struct local_file_header {
				local_file_header() {
					memset(this, 0x00, sizeof(struct local_file_header));
					signature = 0x04034B50;
				}
				local_file_header(const global_file_header& h) {
					memset(this, 0x00, sizeof(struct local_file_header));
					signature = 0x04034B50;
					version_needed = h.version_needed;
					flags = h.flags;
					method = h.method;
					last_modified_time = h.last_modified_time;
					last_modified_date = h.last_modified_date;
					crc32 = h.crc32;
					compressed_size = h.compressed_size;
					uncompressed_size = h.uncompressed_size;
					filename_length = h.filename_length;
					extra_length = h.extra_length;
				}

				uint32_t signature;
				uint16_t version_needed;
				uint16_t flags;
				uint16_t method;
				uint16_t last_modified_time;
				uint16_t last_modified_date;
				uint32_t crc32;
				uint32_t compressed_size;
				uint32_t uncompressed_size;
				uint16_t filename_length;
				uint16_t extra_length;
				// Filename
				// Extra
#ifndef _MSC_VER
			} __attribute__((__packed__));
#else
			};
#endif

			struct data_descriptor {
				data_descriptor() {
					memset(this, 0x00, sizeof(struct data_descriptor));
					signature = 0x08074b50;
				}
				data_descriptor(const struct local_file_header& h) {
					signature = 0x08074b50;
					crc32 = h.crc32;
					compressed_size = h.compressed_size;
					uncompressed_size = h.uncompressed_size;
				}
				data_descriptor(const struct global_file_header& h) {
					signature = 0x08074b50;
					crc32 = h.crc32;
					compressed_size = h.compressed_size;
					uncompressed_size = h.uncompressed_size;
				}

				uint32_t signature;
				uint32_t crc32;
				uint32_t compressed_size;
				uint32_t uncompressed_size;
#ifndef _MSC_VER
			} __attribute__((__packed__));
#else
			};
#endif

			static_assert(sizeof(end_record) == 22, "Invalid struct size");
			static_assert(sizeof(global_file_header) == 46, "Invalid struct size");
			static_assert(sizeof(local_file_header) == 30, "Invalid struct size");
			static_assert(sizeof(data_descriptor) == 16, "Invalid struct size");

#ifdef _MSC_VER
#pragma pack(pop)
#endif
		}
	}
}
