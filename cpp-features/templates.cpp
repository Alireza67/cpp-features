#include "pch.h"
#include <memory>

template<typename T, size_t Size>
std::unique_ptr<T[]> ArrayGenerator()
{
	return std::make_unique<T[]>(Size);
}

TEST(NonType, integral)
{
	auto res = ArrayGenerator<int, 100>();
}
