#pragma once
#include <ostream>
#include <streambuf>
#include <vector>
#include <array>
#include "deflater.h"

namespace thalhammer {
	namespace io {
		class deflate_ostreambuf : public std::streambuf {
			std::vector<char> obuf;
			std::ostream& sink;

			deflater compressor;
		public:
			deflate_ostreambuf(std::ostream& ostream, int level = 9, int windowBits = 15, deflater::wrapper w = deflater::wrapper::zlib, int memlevel = 8, deflater::strategy strat = deflater::strategy::default, size_t bufsize = 4096)
				: obuf(bufsize), sink(ostream), compressor(level, windowBits, w, memlevel, strat)
			{
				if (bufsize <= 1)
					throw std::invalid_argument("buffer size must be larger than 1");
				setp(obuf.data(), obuf.data(), obuf.data() + obuf.size() - 1);
			}

			~deflate_ostreambuf() {
				finish();
			}

			int sync(bool flush) {
				ptrdiff_t n = pptr() - pbase();
				pbump(-n);

				compressor.set_input((const uint8_t*)pbase(), n);
				// Compress and write
				std::array<char, 1024> buf;
				size_t written = 0, read = 0;
				compressor.set_output((uint8_t*)buf.data(), buf.size());
				while (!compressor.need_input()) {
					if (!compressor.compress(read, written, flush))
						return -1;
					sink.write(buf.data(), written);
					compressor.set_output((uint8_t*)buf.data(), buf.size());
				}
				return 0;
			}

			void finish() {
				ptrdiff_t n = pptr() - pbase();
				pbump(-n);

				compressor.finish();
				compressor.set_input((const uint8_t*)pbase(), n);
				// Compress and write
				std::array<char, 1024> buf;
				size_t written = 0, read = 0;
				compressor.set_output((uint8_t*)buf.data(), buf.size());
				while (!compressor.finished()) {
					if (!compressor.compress(read, written))
						return;
					sink.write(buf.data(), written);
					compressor.set_output((uint8_t*)buf.data(), buf.size());
				}
			}
		private:
			int_type overflow(int_type ch) override {
				if (sink && ch != traits_type::eof()) {
					*pptr() = ch;
					pbump(1);
					if (sync(false) == 0) return ch;
				}
				return traits_type::eof();
			}

			int sync() override {
				return sync(true);
			}
		};

		// Write your plain buffer into a compressed stream
		class deflate_ostream : public std::ostream, private deflate_ostreambuf {
		public:
			deflate_ostream(std::ostream& sink, int level = 9, size_t bufsize = 4096)
				: deflate_ostreambuf(sink, level, bufsize), std::ostream(this)
			{
			}

			void finish() {
				deflate_ostreambuf::finish();
			}
		};
	}
}
