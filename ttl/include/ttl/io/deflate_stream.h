#pragma once
#include <ostream>
#include <streambuf>
#include <vector>
#include <array>
#include <ostream>
#include <istream>
#include <cassert>
#include <cstddef>
#include "deflater.h"

namespace ttl {
	namespace io {
		class deflate_ostreambuf : public std::streambuf {
			std::vector<char> obuf;
			std::ostream& sink;

			deflater compressor;
		public:
			deflate_ostreambuf(std::ostream& ostream, int level = 9, int windowBits = 15, deflater::wrapper w = deflater::wrapper::zlib, int memlevel = 8, deflater::strategy strat = deflater::strategy::default_strategy, size_t bufsize = 4096)
				: obuf(bufsize), sink(ostream), compressor(level, windowBits, w, memlevel, strat)
			{
				if (bufsize <= 1)
					throw std::invalid_argument("buffer size must be larger than 1");
				setp(obuf.data(), obuf.data() + obuf.size() - 1);
			}

			~deflate_ostreambuf() {
				finish();
			}

			int sync(bool flush) {
				ptrdiff_t n = pptr() - pbase();
				pbump((int)-n);

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
				pbump((int)-n);

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
		class deflate_ostream : private deflate_ostreambuf, public std::ostream {
		public:
			deflate_ostream(std::ostream& sink, int level = 9, int windowBits = 15, deflater::wrapper w = deflater::wrapper::zlib, int memlevel = 8, deflater::strategy strat = deflater::strategy::default_strategy, size_t bufsize = 4096)
				: deflate_ostreambuf(sink, level, windowBits, w, memlevel, strat, bufsize), std::ostream(this)
			{
			}

			void finish() {
				deflate_ostreambuf::finish();
			}
		};

		class deflate_istreambuf : public std::streambuf {
			size_t put_back;
			size_t readsize;
			std::vector<char> buf;
			std::istream& source;

			deflater compressor;
		public:
			deflate_istreambuf(std::istream& istream, int level = 9, int windowBits = 15, deflater::wrapper w = deflater::wrapper::zlib, int memlevel = 8, deflater::strategy strat = deflater::strategy::default_strategy, size_t preadsize = 4096, size_t pback = 8)
				: put_back(std::max(pback, size_t(1))), readsize(preadsize), buf(put_back), source(istream), compressor(level, windowBits, w, memlevel, strat)
			{
				if (readsize <= 1)
					throw std::invalid_argument("readsize must be larger than 1");
				auto end = buf.data() + buf.size();
				setg(buf.data(), end, end);
			}

			~deflate_istreambuf() {
			}

		private:
			std::streambuf::int_type underflow() override {
				if (gptr() < egptr())
					return traits_type::to_int_type(*gptr());

				assert(gptr() == egptr());
				if (buf.size() == put_back)
					buf.resize(put_back + readsize * 2);

				std::vector<char> rbuf(readsize);
				rbuf.resize(source.read(rbuf.data(), rbuf.size()).gcount());
				if (!source)
					compressor.finish();
				compressor.set_input(reinterpret_cast<uint8_t*>(rbuf.data()), rbuf.size());
				compressor.set_output((uint8_t*)buf.data() + put_back, buf.size() - put_back);
				size_t twritten = 0;
				while (!compressor.need_input()) {
					size_t read, written;
					compressor.compress(read, written);
					twritten += written;
					if (compressor.need_output()) {
						auto osize = buf.size();
						buf.resize(osize*1.5);
						compressor.set_output((uint8_t*)buf.data() + osize, buf.size() - osize);
					}
				}
				buf.resize(twritten + put_back);
				if (twritten == 0 && compressor.finished())
					return traits_type::eof();
				setg(buf.data(), buf.data() + put_back, buf.data() + buf.size());
				return traits_type::to_int_type(*gptr());
			}
		};

		// Read compressed data from a plain istream
		class deflate_istream : private deflate_istreambuf, public std::istream {
		public:
			deflate_istream(std::istream& source, int level = 9, int windowBits = 15, deflater::wrapper w = deflater::wrapper::zlib, int memlevel = 8, deflater::strategy strat = deflater::strategy::default_strategy, size_t bufsize = 4096, size_t pback = 8)
				: deflate_istreambuf(source, level, windowBits, w, memlevel, strat, bufsize, pback), std::istream(this)
			{
			}
		};
	}
}

namespace thalhammer = ttl;