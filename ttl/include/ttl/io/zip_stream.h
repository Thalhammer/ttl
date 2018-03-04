#pragma once
#include <string>
#include <vector>
#include <array>
#include "zip_internals.h"
#include "../crc.h"
#include "zip_entry.h"
#include "deflater.h"

namespace thalhammer {
	namespace io {
		template<bool SupportCompression = true>
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

			inline void write_stream(std::istream& fstream, uint32_t& compressed_size, uint32_t& uncompressed_size, uint32_t& crc, bool compress);
		public:
			explicit zip_stream(std::ostream& pstream)
				: stream(pstream), written(0)
			{}

			void add_entry(zip_entry entry, std::istream& fstream) {
				if (!SupportCompression && entry.is_compressed())
					throw std::logic_error("compression not supported");
				if (entry.is_compressed() && entry.get_compression_method() != zip_internals::compression_method::deflate)
					throw std::logic_error("only deflate compression is supported");
				auto& gheader = entry.header;
				gheader.relative_header_offset = (uint32_t)written;
				{
					zip_internals::local_file_header fheader(gheader);
					stream.write((const char*)&fheader, sizeof(fheader));
					stream.write(entry.name.data(), entry.name.size());
					written += sizeof(fheader) + entry.name.size();
				}
				uint32_t csize, usize, crc;
				write_stream(fstream, csize, usize, crc, entry.is_compressed());
				gheader.compressed_size = csize;
				gheader.uncompressed_size = usize;
				gheader.crc32 = crc;
				{					
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

		template<>
		inline void zip_stream<true>::write_stream(std::istream& fstream, uint32_t& compressed_size, uint32_t& uncompressed_size, uint32_t& crc, bool compress) {
			std::array<char, 4096> readbuf;
			CRC_32 gen_crc;

			compressed_size = 0;
			uncompressed_size = 0;

			if (!compress)
			{
				while (fstream) {
					auto read = fstream.read(readbuf.data(), readbuf.size()).gcount();
					gen_crc.update(reinterpret_cast<uint8_t*>(readbuf.data()), read);
					uncompressed_size += (uint32_t)read;
					compressed_size += (uint32_t)read;
					stream.write(readbuf.data(), read);
				}
			}
			else {
				std::array<char, 4096> compressbuf;
				// Best compression, 32K Window, no zlib header
				deflater zlib(9, 15, deflater::wrapper::none);
				while (fstream) {
					auto read = fstream.read(readbuf.data(), readbuf.size()).gcount();
					gen_crc.update(reinterpret_cast<uint8_t*>(readbuf.data()), read);
					uncompressed_size += (uint32_t)read;

					zlib.set_input(reinterpret_cast<uint8_t*>(readbuf.data()), read);
					while (!zlib.need_input()) {
						size_t zread = 0, zwritten = 0;
						zlib.set_output(reinterpret_cast<uint8_t*>(compressbuf.data()), compressbuf.size());
						if (!zlib.compress(zread, zwritten))
							throw std::runtime_error("zlib error");
						compressed_size += (uInt)zwritten;
						stream.write(compressbuf.data(), zwritten);
					}
				}
				zlib.finish();
				while (!zlib.finished()) {
					size_t zread = 0, zwritten = 0;
					zlib.set_output(reinterpret_cast<uint8_t*>(compressbuf.data()), compressbuf.size());
					if (!zlib.compress(zread, zwritten))
						throw std::runtime_error("zlib error");
					compressed_size += (uInt)zwritten;
					stream.write(compressbuf.data(), zwritten);
				}
			}
			crc = gen_crc.finalize();
		}

		template<>
		inline void zip_stream<false>::write_stream(std::istream& fstream, uint32_t& compressed_size, uint32_t& uncompressed_size, uint32_t& crc, bool compress) {
			std::array<char, 4096> readbuf;
			CRC_32 gen_crc;

			compressed_size = 0;
			uncompressed_size = 0;
			while (fstream) {
				auto read = fstream.read(readbuf.data(), readbuf.size()).gcount();
				gen_crc.update(reinterpret_cast<uint8_t*>(readbuf.data()), read);
				uncompressed_size += (uint32_t)read;
				compressed_size += (uint32_t)read;
				stream.write(readbuf.data(), read);
			}
			crc = gen_crc.finalize();
		}
	}
}
