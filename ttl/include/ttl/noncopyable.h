#pragma once

namespace ttl
{
	struct noncopyable {
		noncopyable & operator=(const noncopyable&) = delete;
		noncopyable(const noncopyable&) = delete;
		noncopyable() = default;
	};
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
