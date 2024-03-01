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

TEST(ranges, all)
{
    auto data{ std::vector{1,2,3,4,5,6,7,8,9} };
    auto view_all = data | std::views::all;

    for (auto i{ 0 }; auto & item: view_all)
    {
        EXPECT_EQ(item, data[i]);
        EXPECT_EQ(&item, &data[i]);
        i++;
    }
}

TEST(ranges, filter)
{
    auto even = [](auto& i) { return i % 2 == 0; };
    auto data{ std::vector{0,1,2,3,4,5,6,7,8,9} };
    auto view_all = data | std::views::filter(even);

    for (auto i{ 0 }; auto & item: view_all)
    {
        EXPECT_EQ(item, data[i]);
        EXPECT_EQ(&item, &data[i]);
        i += 2;
    }
}

TEST(ranges, transform)
{
    auto square = [](auto& i) { return i * i; };
    auto data{ std::vector{0,1,2,3,4,5,6,7,8,9} };
    auto transform_ = data | std::views::transform(square);

    for (auto i{ 0 }; auto item: transform_)
    {
        EXPECT_EQ(item, data[i] * data[i]);
        EXPECT_NE(&item, &data[i]);
        i++;
    }
}

TEST(ranges, drop)
{
    auto data{ std::vector{0,1,2,3,4,5,6,7,8,9} };
    auto drop_ = data | std::views::drop(5);

    for (auto i{ 5 }; auto& item: drop_)
    {
        EXPECT_EQ(item, data[i]);
        EXPECT_EQ(&item, &data[i]);
        i++;
    }
}

TEST(ranges, dropwhile)
{
    auto condition = [](auto& i) {return i < 5; };
    auto data{ std::vector{0,1,2,3,4,5,6,7,8,9} };
    auto dropwhile_ = data | std::views::drop_while(condition);

    EXPECT_EQ(5, dropwhile_.size());
    for (auto i{ 5 }; auto & item: dropwhile_)
    {
        EXPECT_EQ(item, data[i]);
        EXPECT_EQ(&item, &data[i]);
        i++;
    }
}

TEST(ranges, take)
{
    auto data{ std::vector{0,1,2,3,4,5,6,7,8,9} };
    auto take_ = data | std::views::take(3);

    EXPECT_EQ(3, take_.size());
    for (auto i{ 0 }; auto & item: take_)
    {
        EXPECT_EQ(item, data[i]);
        EXPECT_EQ(&item, &data[i]);
        i++;
    }
}

TEST(ranges, takewhile)
{
    auto condition = [](auto& i) {return i < 3; };
    auto data{ std::vector{0,1,2,3,4,5,6,7,8,9} };
    auto takewhile_ = data | std::views::take_while(condition);

    auto i{ 0 };
    for (auto & item: takewhile_)
    {
        EXPECT_EQ(item, data[i]);
        EXPECT_EQ(&item, &data[i]);
        i++;
    }
    EXPECT_EQ(3, i);
}

TEST(ranges, reverse)
{
    auto data{ std::vector{0,1,2,3,4,5,6,7,8,9} };
    auto reverse_ = data | std::views::reverse;

    auto i{ data.size() - 1};
    for (auto& item : reverse_)
    {
        EXPECT_EQ(item, data[i]);
        EXPECT_EQ(&item, &data[i]);
        i--;
    }
}
