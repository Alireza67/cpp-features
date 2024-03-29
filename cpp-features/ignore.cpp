#include "pch.h"
#include <tuple>

std::tuple<int, std::string> GetInfo(int input)
{
	return {10 + input , "This is a simple test!"s};
}

TEST(ignore, check)
{
	auto [a, b] = GetInfo(40);
	EXPECT_EQ(50, a);
	std::tie(a, std::ignore) = GetInfo(50);
	EXPECT_EQ(60, a);
}
