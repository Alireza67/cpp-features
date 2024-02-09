#include "pch.h"
#include <ranges>

TEST(ranges, empty)
{
    std::vector<int> nums = { 1, 2, 3, 4, 5 };
    auto even_nums = nums | std::views::filter([](int& n) {
        return n % 2 == 0;
        });

    EXPECT_FALSE(even_nums.empty());
}

TEST(ranges, begin_end)
{
    std::vector<int> nums = { 1, 2, 3, 4, 5 };
    auto even_nums = nums | std::views::filter([](int& n) {
        return n % 2 == 0;
        });

    EXPECT_EQ(2, *even_nums.begin());
    EXPECT_EQ(4, *(--even_nums.end()));
}

TEST(ranges, back_front)
{
    std::vector<int> nums = { 1, 2, 3, 4, 5 };
    auto even_nums = nums | std::views::filter([](int& n) {
        return n % 2 == 0;
        });

    EXPECT_EQ(2, even_nums.front());
    EXPECT_EQ(4, even_nums.back());
}

