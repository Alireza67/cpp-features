#include "pch.h"
#include <span> 
#include <vector> 

TEST(span, simple)
{
    std::vector<int> target = { 1, 2, 30, 40, 50, 6, 7, 8 };
    std::vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::span<int> span_vec(vec);

    std::span<int> subspan = span_vec.subspan(2, 3);

    for (auto& num : subspan) {
        num *= 10;
    }

    EXPECT_EQ(target, vec);
}
