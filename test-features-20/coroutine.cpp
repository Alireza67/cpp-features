#include "pch.h"
#include <coroutine>

#include <sstream>
#include <iostream>
#include <vector>
#include <experimental/generator>

std::experimental::generator<int> getSequenceGenerator(int startValue, int numberOfValues)
{
    for (int i{ startValue }; i < startValue + numberOfValues; ++i)
    {
        co_yield i;
    }
}

TEST(coroutine, use_of_co_yield)
{
    auto counter{ 0 };
    for (auto n : getSequenceGenerator(0, 5))
    {
        EXPECT_EQ(counter++, n);
    }
}
