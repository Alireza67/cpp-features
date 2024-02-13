#include "pch.h"
#include <string>
#include <tuple>

class LegacyFoo
{
public:

	explicit LegacyFoo(size_t size)
		:size_(size)
	{
		array_ = new int[size_] {};
		for (auto i{0}; i < size_; i++)
		{
			array_[i] = i;
		}
	}

	virtual ~LegacyFoo()
	{
		if (array_)
		{
			delete[] array_;
			array_ = nullptr;
		}
	}

	std::tuple<int*, size_t> GetArray()
	{
		return { array_, size_ };
	}

private:
	int* array_{};
	size_t size_{};
};

void DanglingScenario()
{
	LegacyFoo obj(10);
	auto [dataPtr, dataSize] = obj.GetArray();

	for (auto i{ 0 }; i < dataSize; i++)
	{
		EXPECT_EQ(i, dataPtr[i]);
	}

	LegacyFoo obj2 = obj;
	auto data = obj2.GetArray();
	EXPECT_EQ(dataPtr, std::get<int*>(data));
}
TEST(constructors, without_copy_constructor_dangling_pointer)
{
	EXPECT_DEATH(DanglingScenario(), ".*");
}
