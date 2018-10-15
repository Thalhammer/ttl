#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <array>

namespace ttl
{
	template<typename crc_val, crc_val WIDTH, crc_val Polynomial, crc_val InitialRemainder, crc_val FinalXorValue, bool TReflectData, bool TReflectRemainder>
	class crc
	{
	public:
		typedef crc_val crc_t;
		static inline crc_t get_crc(const std::vector<uint8_t>& data)
		{
			crc c;
			c.update(data);
			return c.finalize();
		}
		static inline crc_t get_crc(const std::string& str)
		{
			crc c;
			c.update(str);
			return c.finalize();
		}
		static inline crc_t get_crc(const uint8_t* data, size_t dlen)
		{
			crc c;
			c.update(data, dlen);
			return c.finalize();
		}

		crc()
			: _table(get_table())
		{
			static_assert(WIDTH >= 8, "Width needs to be at least 8, 5bit crc is not supported.");
			_remainder = (crc_t)InitialRemainder;
		}

		void update(const std::vector<uint8_t>& data)
		{
			this->update(data.data(), data.size());
		}

		void update(const std::string& data)
		{
			this->update((const uint8_t*)data.data(), data.size());
		}

		void update(const uint8_t* idata, size_t dlen)
		{
			for (size_t byte = 0; byte < dlen; byte++)
			{
				uint8_t data = reflect_data(idata[byte]) ^ (_remainder >> (WIDTH - 8));
				_remainder = _table[data] ^ (_remainder << 8);
			}
		}

		template <crc_val x = WIDTH>
		typename std::enable_if<x == sizeof(crc_val)*8, crc_t>::type
			finalize()
		{
			crc_t res = (reflect_remainder(_remainder) ^ FinalXorValue);
			_remainder = (crc_t)InitialRemainder;
			return res;
		}
		
		template <crc_val x = WIDTH>
		typename std::enable_if<x != sizeof(crc_val) * 8, crc_t>::type
			finalize()
		{
			crc_t res = (reflect_remainder(_remainder) ^ FinalXorValue);
			_remainder = (crc_t)InitialRemainder;
			return res & ((1 << WIDTH) - 1);
		}

	private: // Instance members
		crc_t _remainder;
		const std::array<crc_t, 256>& _table;

	private: // Static functions
		template <bool M = TReflectData, typename std::enable_if<M>::type* = nullptr>
		static inline uint8_t reflect_data(uint8_t data)
		{
			return (uint8_t)reflect(data, 8);
		}

		template <bool M = TReflectData, typename std::enable_if<!M>::type* = nullptr>
		static inline uint8_t reflect_data(uint8_t data)
		{
			return data;
		}

		template <bool M = TReflectRemainder, typename std::enable_if<M>::type* = nullptr>
		static inline crc_t reflect_remainder(crc_t data)
		{
			return reflect(data, WIDTH);
		}

		template <bool M = TReflectRemainder, typename std::enable_if<!M>::type* = nullptr>
		static inline crc_t reflect_remainder(crc_t data)
		{
			return data;
		}

		static inline crc_t reflect(crc_t data, uint8_t nBits)
		{
			crc_t  reflection = 0x00000000;
			uint8_t  bit;

			for (bit = 0; bit < nBits; ++bit)
			{
				if (data & 0x01)
					reflection |= (crc_t(1) << ((nBits - 1) - bit));
				data = (data >> 1);
			}

			return reflection;
		}

		static constexpr crc_t TOPBIT = (crc_t(1) << (WIDTH - 1));

		static inline crc_t table_calc_remainder(int32_t dividend)
		{
			crc_t remainder = crc_t(dividend) << (WIDTH - 8);
			for (uint8_t bit = 8; bit > 0; bit--)
			{
				remainder = (remainder&TOPBIT) ? ((remainder << 1) ^ Polynomial) : (remainder << 1);
			}
			return remainder;
		}

		static inline std::array<crc_t, 256> make_table()
		{
			std::array<crc_t, 256> table;
			for (int32_t dividend = 0; dividend < 256; dividend++)
			{
				table[dividend] = table_calc_remainder(dividend);
			}
			return table;
		}

		static inline const std::array<crc_t, 256>& get_table()
		{
			static std::array<crc_t, 256> table = make_table();
			return table;
		}
	};

	typedef crc<uint64_t, 64, 0x42F0E1EBA9EA3693, 0xffffffffffffffff, 0xffffffffffffffff, true, true> CRC_64_XZ;
	typedef crc<uint64_t, 64, 0xAD93D23594C935A9, 0xffffffffffffffff, 0x0000000000000000, true, true> CRC_64_JONES;
	typedef crc<uint64_t, 64, 0x000000000000001B, 0x0000000000000000, 0x0000000000000000, true, true> CRC_64;
	typedef crc<uint32_t, 32, 0x000000AF, 0x00000000, 0x00000000, false, false> CRC_32_XFER;
	typedef crc<uint32_t, 32, 0x04C11DB7, 0xffffffff, 0x00000000, true, true> CRC_32_JAM;
	typedef crc<uint32_t, 32, 0x04C11DB7, 0x00000000, 0xffffffff, false, false> CRC_32_POSIX;
	typedef crc<uint32_t, 32, 0x04C11DB7, 0xffffffff, 0xffffffff, false, false> CRC_32_BZIP2;
	typedef crc<uint32_t, 32, 0x04C11DB7, 0xffffffff, 0x00000000, false, false> CRC_32_MPEG;
	typedef crc<uint32_t, 32, 0x1EDC6F41, 0xffffffff, 0xffffffff, true, true> CRC_32C;
	typedef crc<uint32_t, 32, 0x04C11DB7, 0xffffffff, 0xffffffff, true, true> CRC_32;
	typedef crc<uint32_t, 24, 0x864CFB, 0xb704ce, 0x000000, false, false> CRC_24;
	typedef crc<uint16_t, 16, 0x1021, 0x0000, 0x0000, false, false> CRC_16_XMODEM;
	typedef crc<uint16_t, 16, 0x1021, 0xffff, 0xffff, true, true> CRC_16_X25;
	typedef crc<uint16_t, 16, 0x1021, 0x0000, 0x0000, true, true> CRC_16_KERMIT;
	typedef crc<uint16_t, 16, 0x1021, 0xffff, 0x0000, false, false> CRC_16_CCITT;
	typedef crc<uint16_t, 16, 0x1021, 0xffff, 0xffff, false, false> CRC_16_GENIBUS;
	typedef crc<uint16_t, 16, 0x0589, 0x0000, 0x0001, false, false> CRC_16_R;
	typedef crc<uint16_t, 16, 0x8005, 0xffff, 0x0000, true, true> CRC_16_MODBUS;
	typedef crc<uint16_t, 16, 0x8005, 0xffff, 0xffff, true, true> CRC_16_USB;
	typedef crc<uint16_t, 16, 0x8005, 0x0000, 0x0000, true, true> CRC_16;
	typedef crc<uint16_t, 15, 0x4599, 0x0000, 0x0000, false, false> CRC_15;
	typedef crc<uint16_t, 12, 0x80F, 0x000, 0x000, false, true> CRC_12;
	typedef crc<uint8_t, 8, 0x9B, 0x00, 0x00, true, true> CRC_8_WCDMA;
	typedef crc<uint8_t, 8, 0x07, 0xff, 0x00, true, true> CRC_8_ROHC;
	typedef crc<uint8_t, 8, 0x31, 0x00, 0x00, true, true> CRC_8_DALLAS;
	typedef crc<uint8_t, 8, 0x07, 0x00, 0x55, false, false> CRC_8_ITU;
	typedef crc<uint8_t, 8, 0x1D, 0xfd, 0x00, false, false> CRC_8_ICODE;
	typedef crc<uint8_t, 8, 0x1D, 0xff, 0x00, true, true> CRC_8_EBU;
	typedef crc<uint8_t, 8, 0xD5, 0x00, 0x00, false, false> CRC_8_DVB_S2;
	typedef crc<uint8_t, 8, 0x39, 0x00, 0x00, true, true> CRC_8_DARC;
	typedef crc<uint8_t, 8, 0x9B, 0xff, 0x00, false, false> CRC_8_CDMA2000;
	typedef crc<uint8_t, 8, 0x2F, 0xff, 0xff, false, false> CRC_8_8H2F;
	typedef crc<uint8_t, 8, 0x1D, 0x00, 0x00, false, false> CRC_8_SAE_J1850_ZERO;
	typedef crc<uint8_t, 8, 0x1D, 0xff, 0xff, false, false> CRC_8_SAE_J1850;
	typedef crc<uint8_t, 8, 0x07, 0x00, 0x00, false, false> CRC_8;

	// Alias
	typedef CRC_16_XMODEM CRC_16_ZMODEM;
	typedef CRC_16_XMODEM CRC_16_CCIT_ZERO;
	typedef CRC_8_DALLAS CRC_8_MAXIM;
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
