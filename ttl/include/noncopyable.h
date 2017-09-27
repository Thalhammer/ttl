#pragma once

namespace thalhammer
{
	struct noncopyable {
		noncopyable & operator=(const noncopyable&) = delete;
		noncopyable(const noncopyable&) = delete;
		noncopyable() = default;
	};
}