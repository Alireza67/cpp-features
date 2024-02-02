#include "pch.h"
#include <filesystem>

TEST(filesystem, space)
{
	auto query = std::filesystem::space("C:\\");
	EXPECT_TRUE(query.free);
	EXPECT_TRUE(query.capacity);
	EXPECT_TRUE(query.available);
}