#pragma once
#include <ostream>
#include <streambuf>
#include <vector>
#include <array>
#include "inflater.h"

namespace thalhammer {
	namespace io {
		class inflate_ostreambuf : public std::streambuf {
			std::vector<char> obuf;
			std::ostream& sink;

			inflater decompressor;
		public:
			inflate_ostreambuf(std::ostream& ostream, int windowBits = 15, inflater::wrapper w = inflater::wrapper::zlib, size_t bufsize = 4096)
				: obuf(bufsize), sink(ostream), decompressor(windowBits, w)
			{
				if (bufsize <= 1)
					throw std::invalid_argument("buffer size must be larger than 1");
				setp(obuf.data(), obuf.data() + obuf.size() - 1);
			}

			~inflate_ostreambuf() {
				finish();
			}

			int sync(bool flush) {
				ptrdiff_t n = pptr() - pbase();
				pbump((int)-n);

				decompressor.set_input((const uint8_t*)pbase(), n);
				// decompress and write
				std::array<char, 1024> buf;
				size_t written = 0, read = 0;
				decompressor.set_output((uint8_t*)buf.data(), buf.size());
				while (!decompressor.need_input()) {
					if (!decompressor.uncompress(read, written))
						return -1;
					sink.write(buf.data(), written);
					decompressor.set_output((uint8_t*)buf.data(), buf.size());
				}
				return 0;
			}

			void finish() {
				ptrdiff_t n = pptr() - pbase();
				pbump((int)-n);

				decompressor.set_input((const uint8_t*)pbase(), n);
				// decompress and write
				std::array<char, 1024> buf;
				size_t written = 0, read = 0;
				decompressor.set_output((uint8_t*)buf.data(), buf.size());
				while (!decompressor.need_input()) {
					if (!decompressor.uncompress(read, written))
						return;
					sink.write(buf.data(), written);
					decompressor.set_output((uint8_t*)buf.data(), buf.size());
				}
			}

			bool finished() {
				return decompressor.finished();
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

		// Write your compressed buffer into a plain stream
		class inflate_ostream : private inflate_ostreambuf, public std::ostream {
		public:
			inflate_ostream(std::ostream& sink, int windowBits = 15, inflater::wrapper w = inflater::wrapper::zlib,size_t bufsize = 4096)
				: inflate_ostreambuf(sink, windowBits, w, bufsize), std::ostream(this)
			{
			}

			void finish() {
				inflate_ostreambuf::finish();
			}
		};

		class inflate_istreambuf : public std::streambuf {
			size_t put_back;
			size_t readsize;
			std::vector<char> buf;
			std::istream& source;

			inflater decompressor;
		public:
			inflate_istreambuf(std::istream& istream, int windowBits = 15, inflater::wrapper w = inflater::wrapper::zlib, size_t preadsize = 4096, size_t pback = 8)
				: put_back(std::max(pback, size_t(1))), readsize(preadsize), buf(put_back), source(istream), decompressor(windowBits, w)
			{
				if (readsize <= 1)
					throw std::invalid_argument("readsize must be larger than 1");
				auto end = buf.data() + buf.size();
				setg(buf.data(), end, end);
			}

			~inflate_istreambuf() {
			}

		private:
			std::streambuf::int_type underflow() override {
				if (gptr() < egptr())
					return traits_type::to_int_type(*gptr());

				char *base = buf.data();
				assert(gptr() == egptr());
				if (buf.size() == put_back)
					buf.resize(put_back + readsize * 2);

				std::vector<char> rbuf(readsize);
				rbuf.resize(source.read(rbuf.data(), rbuf.size()).gcount());
				decompressor.set_input(reinterpret_cast<uint8_t*>(rbuf.data()), rbuf.size());
				decompressor.set_output((uint8_t*)buf.data() + put_back, buf.size() - put_back);
				size_t twritten = 0;
				while (!decompressor.need_input()) {
					size_t read, written;
					decompressor.uncompress(read, written);
					twritten += written;
					if (decompressor.need_output()) {
						auto osize = buf.size();
						buf.resize(osize*1.5);
						decompressor.set_output((uint8_t*)buf.data() + osize, buf.size() - osize);
					}
				}
				buf.resize(twritten + put_back);
				if (twritten == 0 && decompressor.finished())
					return traits_type::eof();
				setg(buf.data(), buf.data() + put_back, buf.data() + buf.size());
				return traits_type::to_int_type(*gptr());
			}
		};

		// Read plain data from a compressed istream
		class inflate_istream : private inflate_istreambuf, public std::istream {
		public:
			inflate_istream(std::istream& source, int windowBits = 15, inflater::wrapper w = inflater::wrapper::zlib, size_t bufsize = 4096, size_t pback = 8)
				: inflate_istreambuf(source, windowBits, w, bufsize, pback), std::istream(this)
			{
			}
		};
	}
}
