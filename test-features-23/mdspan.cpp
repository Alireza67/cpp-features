#include "pch.h"
#include <mdspan>
#include <ranges>
#include <vector>
#include <numbers>


TEST(mdspan, _3D)
{
    std::vector<int> data(24, 0);
    std::vector target{ 3, 3, 3, 3, 6, 6, 6, 6,
        9, 9, 9, 9, 6, 6, 6, 6, 12,
        12, 12, 12, 18, 18, 18, 18 };
    auto mat_3D = std::mdspan(data.data(), 2, 3, 4);

    //MSVC does not currently support the multidimensional operator[] language feature
    for (auto x{ 0 }; x != mat_3D.extent(0); ++x)
    {
        for (auto y{ 0 }; y != mat_3D.extent(1); ++y)
        {
            for (auto z{ 0 }; z != mat_3D.extent(2); ++z)
            {
                mat_3D[std::array{ x, y, z }] = 3 * (x + 1) * (y + 1);
            }
        }
    }

    EXPECT_EQ(target, data);
}