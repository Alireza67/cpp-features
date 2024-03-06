#include "pch.h"

template<typename T>
std::list<T> QuickSort(std::list<T> input)
{
	if (input.empty())
	{
		return input;
	}

	std::list<T> result;
	result.splice(result.begin(), input, input.begin());
	T const& pivot = *result.begin();

	auto divide_point = std::partition(input.begin(), input.end(),
		[&](T const& t) {return t < pivot; });

	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	auto new_lower(QuickSort(std::move(lower_part)));
	auto new_higher(QuickSort(std::move(input)));

	result.splice(result.end(), new_higher);
	result.splice(result.begin(), new_lower);

	return result;
}

TEST(quickSort, recursive)
{
	std::list input{ -9, 8, -7, 10, 0, 1};
	std::list target{ -9, -7, 0, 1, 8, 10};
	auto result = QuickSort(input);
	EXPECT_EQ(target, result);
}
