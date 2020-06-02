#include <gtest/gtest.h>
#include "ttl/version.h"

TEST(VersionTest, Compiles) {
	ASSERT_NE(ttl::version_sha1, nullptr);
    auto v = ttl::version_major;
    v = ttl::version_minor;
    v = ttl::version_patch;
    v = ttl::version_commit;
}
