#include "pch.h"
#include <iostream>
#include <type_traits>
#include <iterator>

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


template <typename IteratorType>
std::iterator_traits<IteratorType>::value_type traitTest(IteratorType input)
{
    typedef std::iterator_traits<IteratorType>::value_type ValueType;

    ValueType tmp{};
    tmp = *input;
    return tmp;
}

TEST(typeTraits, valueType)
{
    std::vector input{ 6.6, 3.4 };
    auto result = traitTest(std::begin(input));
    auto epsilon = 1e-6;
    EXPECT_FLOAT_EQ(6.6f, result, epsilon);
}

