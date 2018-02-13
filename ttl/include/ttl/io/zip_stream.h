#pragma once
#include <string>
#include <vector>
#include <array>
#include "zip_internals.h"
#include "../crc.h"
#include "zip_entry.h"

namespace thalhammer {
	namespace io {
		class zip_stream {
			std::vector<zip_entry> files;
			std::ostream& stream;
			size_t written;

			void write_central_directory() {
				zip_internals::end_record edr;
				edr.disk_number = 0;
				edr.central_directory_disk_number = 0;
				edr.num_entries_this_disk = (uint32_t)files.size();
				edr.num_entries = (uint32_t)files.size();
				edr.central_directory_size = 0;
				edr.central_directory_offset = (uint32_t)written;
				edr.zip_comment_length = 0;
				for (auto& e : files) {
					stream.write((const char*)&e.header, sizeof(struct zip_internals::global_file_header));
					stream.write(e.name.data(), e.name.size());
					stream.write(e.extra.data(), e.extra.size());
					stream.write(e.comment.data(), e.comment.size());
					written += sizeof(struct zip_internals::global_file_header) + e.name.size() + e.extra.size() + e.comment.size();
				}
				edr.central_directory_size = (uint32_t)(written - edr.central_directory_offset);
				stream.write((const char*)&edr, sizeof(edr));
			}
		public:
			zip_stream(std::ostream& pstream)
				: stream(pstream), written(0)
			{}

			void add_entry(zip_entry entry, std::istream& fstream) {
				if (entry.is_compressed())
					throw std::logic_error("Compression not yet supported");
				auto& gheader = entry.header;
				gheader.relative_header_offset = (uint32_t)written;
				{
					zip_internals::local_file_header fheader(gheader);
					stream.write((const char*)&fheader, sizeof(fheader));
					stream.write(entry.name.data(), entry.name.size());
					written += sizeof(fheader) + entry.name.size();
				}
				{
					std::array<char, 4096> buf;
					
					gheader.compressed_size = 0;
					CRC_32 crc;
					while (fstream) {
						auto read = fstream.read(buf.data(), buf.size()).gcount();
						stream.write(buf.data(), read);
						gheader.compressed_size += (uint32_t)read;
						gheader.uncompressed_size += (uint32_t)read;
						crc.update(reinterpret_cast<uint8_t*>(buf.data()), read);
					}
					gheader.crc32 = crc.finalize();
					zip_internals::data_descriptor descriptor(gheader);
					stream.write((const char*)&descriptor, sizeof(descriptor));
					written += descriptor.compressed_size + sizeof(descriptor);
				}
				files.emplace_back(std::move(entry));
			}

			void add_entry(zip_entry entry) {
				auto& gheader = entry.header;
				gheader.flags &= ~zip_internals::file_flags::data_descriptor;
				gheader.relative_header_offset = (uint32_t)written;
				{
					zip_internals::local_file_header fheader(gheader);
					stream.write((const char*)&fheader, sizeof(fheader));
					stream.write(entry.name.data(), entry.name.size());
					written += sizeof(fheader) + entry.name.size();
				}
				files.emplace_back(std::move(entry));
			}

			void add_file(const std::string& name, std::istream& fstream) {
				zip_entry entry;
				entry.set_name(name);
				return add_entry(entry, fstream);
			}

			void add_file(const std::string& name, const std::string& content) {
				std::istringstream ss(content);
				return add_file(name, ss);
			}

			void add_directory(const std::string& name) {
				zip_entry entry;
				entry.set_name(name);
				entry.set_compressed(false);
				entry.set_directory(true);
				add_entry(std::move(entry));
			}

			void finish() {
				this->write_central_directory();
				stream.flush();
			}
		};
	}
}
