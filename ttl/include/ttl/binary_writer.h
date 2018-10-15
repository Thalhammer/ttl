#pragma once
#include <ostream>

namespace ttl
{
	class binary_writer {
		std::ostream& _stream;
	public:
		explicit binary_writer(std::ostream& stream)
			: _stream(stream)
		{}

		std::ostream& get_stream() { return _stream; }

		void write(const uint8_t* value, size_t len) {
			_stream.write(reinterpret_cast<const char*>(value), len);
		}

		void write(const uint8_t* value, size_t start, size_t len) {
			write(value + start, len);
		}

		void write(const char* value, size_t len) {
			_stream.write(value, len);
		}

		void write(const char* value, size_t start, size_t len) {
			write(value + start, len);
		}

		void write(char value) {
			write(&value, 1);
		}

		void write(uint8_t value) {
			write(&value, 1);
		}

		void write(bool value) {
			this->write(value ? uint8_t(1) : uint8_t(0));
		}

		void write(double value) {
			write(reinterpret_cast<uint8_t*>(&value), sizeof(double));
		}

		void write(float value) {
			write(reinterpret_cast<uint8_t*>(&value), sizeof(float));
		}

		void write(int16_t value) {
			write(reinterpret_cast<uint8_t*>(&value), sizeof(int16_t));
		}

		void write(int32_t value) {
			write(reinterpret_cast<uint8_t*>(&value), sizeof(int32_t));
		}

		void write(int64_t value) {
			write(reinterpret_cast<uint8_t*>(&value), sizeof(int64_t));
		}

		void write(uint16_t value) {
			write(reinterpret_cast<uint8_t*>(&value), sizeof(uint16_t));
		}

		void write(uint32_t value) {
			write(reinterpret_cast<uint8_t*>(&value), sizeof(uint32_t));
		}

		void write(uint64_t value) {
			write(reinterpret_cast<uint8_t*>(&value), sizeof(uint64_t));
		}

		void writeLEB(uint64_t value) {
			do {
				if (value < 128) {
					write((uint8_t)value);
				}
				else {
					write((uint8_t)(value | 0x80));
				}
				value = value >> 7;
			} while (value != 0);
		}

		void writeLEB(int64_t value) {
			writeLEB((uint64_t)value);
		}

		void write(const std::string& value) {
			writeLEB(value.size());
			write(value.data(), value.size());
		}
	};
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
