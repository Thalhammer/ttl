#pragma once
#include <algorithm> 
#include <cctype>
#include <locale>
#include <limits>
#include <vector>
#include <sstream>

namespace thalhammer
{
	namespace string {
		// String length overload for good old char arrays
		template<size_t Len>
		constexpr inline size_t length(const char (&data)[Len]) noexcept {
			return Len - 1;
		}

		// String length overload for std::basic_string
		template<typename CharT, typename Traits, typename Alloc>
		inline size_t length(const std::basic_string<CharT, Traits, Alloc>& str) noexcept {
			return str.length();
		}

		// trim from start (in place)
		template<typename StringType = std::string>
		inline void ltrim(StringType &s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
		}

		// trim from end (in place)
		template<typename StringType = std::string>
		inline void rtrim(StringType &s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(), s.end());
		}

		// trim from both ends (in place)
		template<typename StringType = std::string>
		inline void trim(StringType &s) {
			ltrim(s);
			rtrim(s);
		}

		// trim from start (copying)
		template<typename StringType = std::string>
		inline StringType ltrim_copy(StringType s) {
			ltrim(s);
			return s;
		}

		// trim from end (copying)
		template<typename StringType = std::string>
		inline StringType rtrim_copy(StringType s) {
			rtrim(s);
			return s;
		}

		// trim from both ends (copying)
		template<typename StringType = std::string>
		inline StringType trim_copy(StringType s) {
			trim(s);
			return s;
		}

		// split string
		template<typename StringType = std::string>
		inline std::vector<StringType> split(const StringType& s, const StringType& delim, size_t max = std::numeric_limits<size_t>::max()) {
			std::vector<StringType> res;
			size_t offset = 0;
			do {
				auto pos = s.find(delim, offset);
				if (res.size() < max - 1 && pos != std::string::npos)
					res.push_back(s.substr(offset, pos - offset));
				else {
					res.push_back(s.substr(offset));
					break;
				}
				offset = pos + delim.size();
			} while (true);
			return res;
		}

		// Join strings using delimiter
		template<typename Iterator, typename Delim = std::string, typename StringType = std::string>
		inline StringType join(Iterator beg, Iterator end, const Delim& delim) {
			StringType res;
			for (; beg != end; beg++) {
				if (!res.empty())
					res += delim;
				res += *beg;
			}
			return res;
		}

		// Shorthand for joining vector of strings
		template<typename Delim = std::string, typename StringType = std::string>
		inline StringType join(const std::vector<StringType>& vect, const Delim& delim) {
			return join(std::cbegin(vect), std::cend(vect), delim);
		}

		// Check if string starts with a given string
		template<typename StringType = std::string>
		inline bool starts_with(const StringType& s, const StringType& start) {
			return s.compare(0, start.size(), start) == 0;
		}

		// Check if string ends with a given string
		template<typename StringType = std::string>
		inline bool ends_with(const StringType& s, const StringType& end) {
			return s.size() > end.size() && s.compare(s.size() - end.size(), end.size(), end) == 0;
		}

		template<typename StringType = std::string>
		inline void to_lower(StringType& s) {
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		}

		template<typename StringType = std::string>
		inline void to_upper(StringType& s) {
			std::transform(s.begin(), s.end(), s.begin(), ::toupper);
		}

		template<typename StringType = std::string>
		inline StringType to_lower_copy(StringType s) {
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);
			return s;
		}

		template<typename StringType = std::string>
		inline StringType to_upper_copy(StringType s) {
			std::transform(s.begin(), s.end(), s.begin(), ::toupper);
			return s;
		}

		template<typename StringType = std::string>
		inline double stod(const StringType& str, const std::locale& loc) {
			double res;
			std::istringstream ss(str);
			ss.imbue(loc);
			ss >> res;
			return res;
		}

		// Replace all occurrences of one string with a other (in place)
		template<typename T, typename TFind, typename TReplace>
		inline void replace(T& str, const TFind& find, const TReplace& replace) {
			auto flen = length(find);
			auto rlen = length(replace);
			if(flen == 0)
				throw std::invalid_argument("Find string can not be empty");
			auto pos = str.find(find);
			while(pos != T::npos) {
				str.replace(pos, flen, replace);
				pos = str.find(find, pos + rlen);
			}
		}

		// Replace all occurrences of one string with a other (copying)
		template<typename T, typename TFind, typename TReplace>
		inline T replace_copy(const T& str, const TFind& find, const TReplace& replace) {
			T copy = str;
			string::replace(copy, find, replace);
			return copy;
		}
	}
}