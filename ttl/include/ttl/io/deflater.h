#pragma once
#include <zlib.h>
#include <stdexcept>
#include <cstring>

namespace ttl {
	namespace io {
		class deflater {
			z_stream zlib_stream;

			bool should_finish = false;
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

			deflater(int level = 9, int windowBits = 15, wrapper w = wrapper::zlib, int memlevel = 8, strategy strat = strategy::default_strategy) {
				if (level < 0 || level > 9)
					throw std::invalid_argument("level out of range");
				if (windowBits < 9 || windowBits > 15)
					throw std::invalid_argument("invalid windowBits");
				if (memlevel < 1 || memlevel > 9)
					throw std::invalid_argument("memlevel out of range");

				memset(&zlib_stream, 0x00, sizeof(z_stream));

				if (w == wrapper::none)
					windowBits = -windowBits;
				else if (w == wrapper::gzip)
					windowBits = windowBits + 16;

				int istrat;
				switch (strat) {
				case strategy::filtered: istrat = Z_FILTERED; break;
				case strategy::huffman_only: istrat = Z_HUFFMAN_ONLY; break;
				case strategy::run_length_encoding: istrat = Z_RLE; break;
				case strategy::default_strategy:
				default:
					istrat = Z_DEFAULT_STRATEGY; break;
				}

				auto res = deflateInit2(&zlib_stream, level, Z_DEFLATED, windowBits, memlevel, istrat);
				if (res == Z_VERSION_ERROR)
					throw std::logic_error("incompatible zlib versions");
				if (res != Z_OK)
					throw std::runtime_error("Failed to init zlib deflate");
			}

			deflater(const deflater& other) {
				memset(&zlib_stream, 0x00, sizeof(z_stream));

				auto res = deflateCopy(&zlib_stream, (z_streamp)&other.zlib_stream);
				if (res != Z_OK)
					throw std::runtime_error("Failed to copy zlib state");
			}

			deflater& operator=(const deflater& other) {
				if (zlib_stream.state != nullptr) {
					if (deflateEnd(&zlib_stream) != Z_OK)
						throw std::runtime_error("Failed to end zlib stream");
				}

				memset(&zlib_stream, 0x00, sizeof(z_stream));

				auto res = deflateCopy(&zlib_stream, (z_streamp)&other.zlib_stream);
				if (res != Z_OK)
					throw std::runtime_error("Failed to copy zlib state");
				return *this;
			}

			~deflater() {
				deflateEnd(&zlib_stream);
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

			bool compress(size_t& read, size_t& written, bool pflush = false) {
				int flush = pflush ? Z_FULL_FLUSH : Z_NO_FLUSH;
				if (should_finish) flush = Z_FINISH;
				
				auto o_in = zlib_stream.avail_in;
				auto o_out = zlib_stream.avail_out;
				
				auto res = deflate(&zlib_stream, flush);
				if (res == Z_STREAM_ERROR) throw std::runtime_error("Stream error");

				read = o_in - zlib_stream.avail_in;
				written = o_out - zlib_stream.avail_out;

				if (res == Z_STREAM_END) {
					is_finished = true;
					return true;
				}
				return res == Z_OK;
			}

			void finish() {
				should_finish = true;
			}

			bool finished() const {
				return is_finished;
			}

			static std::vector<uint8_t> compress(const uint8_t* data, size_t dlen) {
				deflater def;
				return compress(data, dlen, def);
			}
			static std::vector<uint8_t> compress(const uint8_t* data, size_t dlen, deflater& def) {
				std::vector<uint8_t> buf(dlen);
				def.set_input(data, dlen);
				def.set_output(buf.data(), buf.size());
				size_t twritten = 0;
				while (!def.finished()) {
					size_t read = 0, written = 0;
					if (!def.compress(read, written))
						throw std::runtime_error("compress returned false");
					twritten += written;
					if (def.need_input()) def.finish();
					if (def.need_output()) {
						auto osize = buf.size();
						buf.resize(osize*1.5);
						def.set_output(buf.data() + osize, buf.size() - osize);
					}
				}
				buf.resize(twritten);
				buf.shrink_to_fit();
				return buf;
			}
		};
	}
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
