#pragma once

namespace ttl
{
	struct noncopyable {
		noncopyable & operator=(const noncopyable&) = delete;
		noncopyable(const noncopyable&) = delete;
		noncopyable() = default;
	};
}

namespace thalhammer = ttl;