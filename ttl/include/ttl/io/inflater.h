#pragma once
#include <zlib.h>
#include <stdexcept>
#include <cstring>

namespace ttl {
	namespace io {
		class inflater {
			z_stream zlib_stream;

			bool is_finished = false;
		public:
			enum class wrapper {
				none,
				gzip,
				zlib
			};
			enum class strategy {
				default_strategy,
				filtered,
				huffman_only,
				run_length_encoding
			};

			inflater(int windowBits = 15, wrapper w = wrapper::zlib) {
				if (windowBits < 9 || windowBits > 15)
					throw std::invalid_argument("invalid windowBits");

				memset(&zlib_stream, 0x00, sizeof(z_stream));

				if (w == wrapper::none)
					windowBits = -windowBits;
				else if (w == wrapper::gzip)
					windowBits = windowBits + 16;

				auto res = inflateInit2(&zlib_stream, windowBits);
				if (res == Z_VERSION_ERROR)
					throw std::logic_error("incompatible zlib versions");
				if (res != Z_OK)
					throw std::runtime_error("Failed to init zlib inflate");
			}

			inflater(const inflater& other) {
				memset(&zlib_stream, 0x00, sizeof(z_stream));

				auto res = inflateCopy(&zlib_stream, const_cast<z_stream*>(&other.zlib_stream));
				if (res != Z_OK)
					throw std::runtime_error("Failed to copy zlib state");
			}

			inflater& operator=(const inflater& other) {
				if (zlib_stream.state != nullptr) {
					if (inflateEnd(&zlib_stream) != Z_OK)
						throw std::runtime_error("Failed to end zlib stream");
				}

				memset(&zlib_stream, 0x00, sizeof(z_stream));

				auto res = inflateCopy(&zlib_stream, const_cast<z_stream*>(&other.zlib_stream));
				if (res != Z_OK)
					throw std::runtime_error("Failed to copy zlib state");
				return *this;
			}

			~inflater() {
				inflateEnd(&zlib_stream);
			}

			void set_input(const uint8_t* ptr, size_t len) {
				zlib_stream.next_in = const_cast<uint8_t*>(ptr);
				zlib_stream.avail_in = (uInt)len;
			}

			void set_output(uint8_t* ptr, size_t len) {
				zlib_stream.next_out = ptr;
				zlib_stream.avail_out = (uInt)len;
			}

			bool need_input() const {
				return zlib_stream.avail_in == 0;
			}

			bool need_output() const {
				return zlib_stream.avail_out == 0;
			}

			bool uncompress(size_t& read, size_t& written) {

				auto o_in = zlib_stream.avail_in;
				auto o_out = zlib_stream.avail_out;

				auto res = inflate(&zlib_stream, Z_NO_FLUSH);
				if (res == Z_STREAM_ERROR) throw std::runtime_error("Stream error");

				read = o_in - zlib_stream.avail_in;
				written = o_out - zlib_stream.avail_out;

				if (res == Z_STREAM_END) {
					is_finished = true;
					return true;
				}
				return res == Z_OK;
			}

			bool finished() const {
				return is_finished;
			}

			static std::vector<uint8_t> uncompress(const uint8_t* data, size_t dlen) {
				inflater inf;
				return uncompress(data, dlen, inf);
			}

			static std::vector<uint8_t> uncompress(const uint8_t* data, size_t dlen, inflater& inf) {
				std::vector<uint8_t> buf(dlen);
				inf.set_input(data, dlen);
				inf.set_output(buf.data(), buf.size());
				size_t twritten = 0;
				while (!inf.finished()) {
					size_t read = 0, written = 0;
					if (!inf.uncompress(read, written))
						throw std::runtime_error("uncompress returned false");
					twritten += written;
					if (inf.need_output()) {
						auto osize = buf.size();
						buf.resize(osize*1.5);
						inf.set_output(buf.data() + osize, buf.size() - osize);
					}
				}
				buf.resize(twritten);
				buf.shrink_to_fit();
				return buf;
			}
		};
	}
}

namespace thalhammer = ttl;