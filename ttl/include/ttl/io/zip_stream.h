#pragma once
#include <string>
#include <vector>
#include <array>
#include <istream>
#include "zip_internals.h"
#include "../crc.h"
#include "zip_entry.h"
#include "deflater.h"

namespace ttl {
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
				edr.num_entries_this_disk = static_cast<uint16_t>(files.size());
				edr.num_entries = static_cast<uint16_t>(files.size());
				edr.central_directory_size = 0;
				edr.central_directory_offset = static_cast<uint32_t>(written);
				edr.zip_comment_length = 0;
				for (auto& e : files) {
					stream.write(reinterpret_cast<const char*>(&e.m_header), sizeof(struct zip_internals::global_file_header));
					stream.write(e.m_name.data(), static_cast<std::streamsize>(e.m_name.size()));
					stream.write(e.m_extra.data(), static_cast<std::streamsize>(e.m_extra.size()));
					stream.write(e.m_comment.data(), static_cast<std::streamsize>(e.m_comment.size()));
					written += sizeof(struct zip_internals::global_file_header) + e.m_name.size() + e.m_extra.size() + e.m_comment.size();
				}
				edr.central_directory_size = static_cast<uint32_t>(written - edr.central_directory_offset);
				stream.write(reinterpret_cast<const char*>(&edr), sizeof(edr));
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
				auto& gheader = entry.m_header;
				gheader.relative_header_offset = static_cast<uint32_t>(written);
				{
					zip_internals::local_file_header fheader(gheader);
					stream.write(reinterpret_cast<const char*>(&fheader), sizeof(fheader));
					stream.write(entry.m_name.data(), static_cast<std::streamsize>(entry.m_name.size()));
					written += sizeof(fheader) + entry.m_name.size();
				}
				uint32_t csize, usize, crc;
				write_stream(fstream, csize, usize, crc, entry.is_compressed());
				gheader.compressed_size = csize;
				gheader.uncompressed_size = usize;
				gheader.crc32 = crc;
				{					
					zip_internals::data_descriptor descriptor(gheader);
					stream.write(reinterpret_cast<const char*>(&descriptor), sizeof(descriptor));
					written += descriptor.compressed_size + sizeof(descriptor);
				}
				files.emplace_back(std::move(entry));
			}

			void add_entry(zip_entry entry) {
				auto& gheader = entry.m_header;
				gheader.flags &= ~zip_internals::file_flags::data_descriptor;
				gheader.relative_header_offset = static_cast<uint32_t>(written);
				{
					zip_internals::local_file_header fheader(gheader);
					stream.write(reinterpret_cast<const char*>(&fheader), sizeof(fheader));
					stream.write(entry.m_name.data(), static_cast<std::streamsize>(entry.m_name.size()));
					written += sizeof(fheader) + entry.m_name.size();
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
					gen_crc.update(reinterpret_cast<uint8_t*>(readbuf.data()), static_cast<size_t>(read));
					uncompressed_size += static_cast<uint32_t>(read);
					compressed_size += static_cast<uint32_t>(read);
					stream.write(readbuf.data(), read);
				}
			}
			else {
				std::array<char, 4096> compressbuf;
				// Best compression, 32K Window, no zlib header
				deflater zlib(9, 15, deflater::wrapper::none);
				while (fstream) {
					auto read = fstream.read(readbuf.data(), readbuf.size()).gcount();
					gen_crc.update(reinterpret_cast<uint8_t*>(readbuf.data()), static_cast<size_t>(read));
					uncompressed_size += static_cast<uint32_t>(read);

					zlib.set_input(reinterpret_cast<uint8_t*>(readbuf.data()), static_cast<size_t>(read));
					while (!zlib.need_input()) {
						size_t zread = 0, zwritten = 0;
						zlib.set_output(reinterpret_cast<uint8_t*>(compressbuf.data()), compressbuf.size());
						if (!zlib.compress(zread, zwritten))
							throw std::runtime_error("zlib error");
						compressed_size += static_cast<uInt>(zwritten);
						stream.write(compressbuf.data(), static_cast<std::streamsize>(zwritten));
					}
				}
				zlib.finish();
				while (!zlib.finished()) {
					size_t zread = 0, zwritten = 0;
					zlib.set_output(reinterpret_cast<uint8_t*>(compressbuf.data()), compressbuf.size());
					if (!zlib.compress(zread, zwritten))
						throw std::runtime_error("zlib error");
					compressed_size += static_cast<uint32_t>(zwritten);
					stream.write(compressbuf.data(), static_cast<std::streamsize>(zwritten));
				}
			}
			crc = gen_crc.finalize();
		}

		template<>
		inline void zip_stream<false>::write_stream(std::istream& fstream, uint32_t& compressed_size, uint32_t& uncompressed_size, uint32_t& crc, bool) {
			std::array<char, 4096> readbuf;
			CRC_32 gen_crc;

			compressed_size = 0;
			uncompressed_size = 0;
			while (fstream) {
				auto read = fstream.read(readbuf.data(), readbuf.size()).gcount();
				gen_crc.update(reinterpret_cast<uint8_t*>(readbuf.data()), static_cast<size_t>(read));
				uncompressed_size += static_cast<uint32_t>(read);
				compressed_size += static_cast<uint32_t>(read);
				stream.write(readbuf.data(), read);
			}
			crc = gen_crc.finalize();
		}
	}
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
