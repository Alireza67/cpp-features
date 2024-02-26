#include "pch.h"
#include <iostream>
#include <type_traits>
#include <list>
#include <queue>
#include <iterator>
#include <forward_list>

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

template<typename IteratorType>
std::iterator_traits<IteratorType>::difference_type CalculateRange(IteratorType pointA, IteratorType pointB)
{
    typedef std::iterator_traits<IteratorType>::difference_type DiffType;
    DiffType a = *pointA;
    DiffType b = *pointB;
    return abs(pointA - pointB);
}

TEST(typeTraits, difference_type)
{
    std::vector input{3.3, 2.1, 4.5, 8.4};
    auto result = CalculateRange(std::begin(input), --std::end(input));
    EXPECT_FLOAT_EQ(3, result);
}

template <typename IteratorType>
std::string CheckCategory(IteratorType begin)
{
    typedef std::iterator_traits<IteratorType>::iterator_category Category;

    if (std::is_same_v<Category, std::random_access_iterator_tag>) {
        return "Random Access";
    }
    else if (std::is_same_v<Category, std::bidirectional_iterator_tag>) {
        return "Bidirectional";
    }
    else if (std::is_same_v<Category, std::forward_iterator_tag>) {
        return "Forward";
    }
    else if (std::is_same_v<Category, std::input_iterator_tag>) {
        return "Input";
    }
    else {
        return "Unknown Category!";
    }
}

TEST(typeTraits, iterator_category)
{
    std::vector data{3.3};
    EXPECT_EQ("Random Access"s, CheckCategory(std::begin(data)));
    std::list data2{ 2.2 };
    EXPECT_EQ("Bidirectional"s, CheckCategory(std::begin(data2)));
    std::forward_list<double> data3{ 1.1 };
    EXPECT_EQ("Forward"s, CheckCategory(data3.begin()));
    std::istream_iterator<double> data4(std::cin);
    EXPECT_EQ("Input"s, CheckCategory(data4));
}
