#include "pch.h"
#include <iostream>
#include <type_traits>

template <typename T>
std::string CheckType() {
    if (std::is_integral<T>::value) {
        return "Type is integral"s;
    }
    if (std::is_reference<T>::value) {
        return "Type is reference"s;
    }
    else if (std::is_floating_point<T>::value) {
        return "Type is floating point"s;
    }
    else {
        return "Type is neither integral nor floating point"s;
    }
}

TEST(typeTraits, checkType)
{
    int x = 10;
    EXPECT_EQ("Type is integral"s, CheckType<decltype(x)>());

    int& y = x;
    EXPECT_EQ("Type is reference"s, CheckType<decltype(y)>());

    auto z = 9.5f;
    EXPECT_EQ("Type is floating point"s, CheckType<decltype(z)>());

    std::string xyz {"xyz"s};
    EXPECT_EQ("Type is neither integral nor floating point"s, CheckType<decltype(xyz)>());
}
