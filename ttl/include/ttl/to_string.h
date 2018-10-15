#pragma once
#include <string>

namespace ttl
{
	using std::to_string;

	inline std::string to_string(std::string str) {
		return str;
	}

	// You can add custom to_string functions to this
	// namespace to make them available for any::to_string.
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
