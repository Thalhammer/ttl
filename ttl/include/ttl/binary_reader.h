#pragma once
#include <istream>

namespace ttl
{
	class binary_reader {
		std::istream& _stream;

		void read_block(uint8_t* ptr, size_t len) {
			_stream.read(reinterpret_cast<char*>(ptr), static_cast<std::streamsize>(len));
			if (!_stream) throw std::runtime_error("unexpected end of file");
		}

		template<typename T>
		T read_scalar() {
			T res;
			read_block(reinterpret_cast<uint8_t*>(&res), sizeof(T));
			return res;
		}
	public:
		explicit binary_reader(std::istream& stream)
			: _stream(stream)
		{}

		std::istream& get_stream() { return _stream; }

		uint8_t read_uint8() { return read_scalar<uint8_t>(); }
		int8_t read_int8() { return read_scalar<int8_t>(); }
		uint16_t read_uint16() { return read_scalar<uint16_t>(); }
		int16_t read_int16() { return read_scalar<int16_t>(); }
		uint32_t read_uint32() { return read_scalar<uint32_t>(); }
		int32_t read_int32() { return read_scalar<int32_t>(); }
		uint64_t read_uint64() { return read_scalar<uint64_t>(); }
		int64_t read_int64() { return read_scalar<int64_t>(); }
		double read_double() { return read_scalar<double>(); }
		float read_float() { return read_scalar<float>(); }
		bool read_bool() { return read_uint8() != 0; }

		uint64_t read_unsigned_LEB() {
			uint64_t res = 0;
			uint8_t t = read_uint8();
			int shift = 0;
			while (t & 0x80) {
				res |= uint64_t(t & 0x7f) << shift;
				t = read_uint8();
				shift += 7;
			}
			res |= uint64_t(t) << shift;
			return res;
		}
		int64_t read_LEB() {
			return static_cast<int64_t>(read_unsigned_LEB());
		}

		std::string read_string() {
			auto size = read_unsigned_LEB();
			std::string res;
			res.resize(size);
			read_block(reinterpret_cast<uint8_t*>(const_cast<char*>(res.data())), size);
			return res;
		}
	};
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
