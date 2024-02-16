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

	for (auto i{ 0 }; i < dataSize; i++)
	{
		dataPtr[i] *= 2;
		EXPECT_EQ(dataPtr[i], std::get<int*>(data)[i]);
	}
}

TEST(constructors, without_copy_constructor_dangling_pointer)
{
	EXPECT_DEATH(DanglingScenario(), ".*");
}

class LegacyFoo2
{
public:

	explicit LegacyFoo2(size_t size)
		:size_(size)
	{
		array_ = new int[size_] {};
		for (auto i{ 0 }; i < size_; i++)
		{
			array_[i] = i;
		}
	}

	LegacyFoo2(const LegacyFoo2& rhs)
		:LegacyFoo2(rhs.size_)
	{
		for (auto i{ 0 }; i < size_; i++)
		{
			array_[i] = rhs.array_[i];
		}
	}

	virtual ~LegacyFoo2()
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

TEST(constructors, with_copy_constructor)
{
	LegacyFoo2 obj(10);
	auto [dataPtr, dataSize] = obj.GetArray();

	for (auto i{ 0 }; i < dataSize; i++)
	{
		EXPECT_EQ(i, dataPtr[i]);
	}

	auto obj2 = obj;
	auto data = obj2.GetArray();
	EXPECT_NE(dataPtr, std::get<int*>(data));
}

class LegacyFoo3
{
public:

	explicit LegacyFoo3(size_t size)
		:size_(size)
	{
		array_ = new int[size_] {};
		for (auto i{ 0 }; i < size_; i++)
		{
			array_[i] = i;
		}
	}

	LegacyFoo3(const LegacyFoo3& rhs)
		:LegacyFoo3(rhs.size_)
	{
		for (auto i{ 0 }; i < size_; i++)
		{
			array_[i] = rhs.array_[i];
		}
	}

	LegacyFoo3(LegacyFoo3&& rhs) noexcept
	{
		array_ = std::exchange(rhs.array_, nullptr);
		size_ = std::exchange(rhs.size_, 0);
	}

	virtual ~LegacyFoo3()
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

TEST(constructors, move_constructor)
{
	LegacyFoo3 obj(10);
	auto [dataPtr, dataSize] = obj.GetArray();

	for (auto i{ 0 }; i < dataSize; i++)
	{
		dataPtr[i] = 5 * i;
		EXPECT_EQ(5 * i, dataPtr[i]);
	}

	auto obj2(std::move(obj));
	auto [dataPtr2, dataSize2] = obj2.GetArray();
	EXPECT_EQ(10, dataSize2);
	EXPECT_EQ(dataPtr, dataPtr2);

	for (auto i{ 0 }; i < dataSize2; i++)
	{
		EXPECT_EQ(5 * i, dataPtr2[i]);
	}
}

class LegacyFoo4
{
public:

	explicit LegacyFoo4(size_t size)
		:size_(size)
	{
		array_ = new int[size_] {};
		for (auto i{ 0 }; i < size_; i++)
		{
			array_[i] = i;
		}
	}

	LegacyFoo4(const LegacyFoo4& rhs)
		:LegacyFoo4(rhs.size_)
	{
		for (auto i{ 0 }; i < size_; i++)
		{
			array_[i] = rhs.array_[i];
		}
	}

	LegacyFoo4(LegacyFoo4&& rhs) noexcept
	{
		array_ = std::exchange(rhs.array_, nullptr);
		size_ = std::exchange(rhs.size_, 0);
	}

	LegacyFoo4& operator=(const LegacyFoo4& rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}

		size_ = 0;
		delete[] array_;
		array_ = nullptr;

		auto tmp = rhs;
		std::swap(array_, tmp.array_);
		std::swap(size_, tmp.size_);
		return *this;
	}

	virtual ~LegacyFoo4()
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

TEST(constructors, copy_assignment)
{
	LegacyFoo4 obj(10);
	obj = obj;
	auto result1 = obj.GetArray();
	for (auto i{ 0 }; i < std::get<size_t>(result1); i++)
	{
		std::get<int*>(result1)[i] = 5 * i;
	}

	LegacyFoo4 obj2(5);
	obj2 = obj;
	auto result2 = obj2.GetArray();

	for (auto i{ 0 }; i < std::get<size_t>(result2); i++)
	{
		EXPECT_EQ(5 * i, std::get<int*>(result2)[i]);
	}
}

class LegacyFoo5
{
public:

	explicit LegacyFoo5(size_t size)
		:size_(size)
	{
		array_ = new int[size_] {};
		for (auto i{ 0 }; i < size_; i++)
		{
			array_[i] = i;
		}
	}

	LegacyFoo5(const LegacyFoo5& rhs)
		:LegacyFoo5(rhs.size_)
	{
		for (auto i{ 0 }; i < size_; i++)
		{
			array_[i] = rhs.array_[i];
		}
	}

	LegacyFoo5(LegacyFoo5&& rhs) noexcept
	{
		array_ = std::exchange(rhs.array_, nullptr);
		size_ = std::exchange(rhs.size_, 0);
	}

	LegacyFoo5& operator=(const LegacyFoo5& rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}

		size_ = 0;
		delete[] array_;
		array_ = nullptr;

		auto tmp = rhs;
		std::swap(array_, tmp.array_);
		std::swap(size_, tmp.size_);
		return *this;
	}

	LegacyFoo5& operator=(LegacyFoo5&& rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}

		size_ = 0;
		delete[] array_;
		array_ = nullptr;

		array_ = std::exchange(rhs.array_, nullptr);
		size_ = std::exchange(rhs.size_, 0);
		return *this;
	}

	virtual ~LegacyFoo5()
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

TEST(constructors, move_assignment)
{
	LegacyFoo5 obj(10);
	obj = obj;
	auto result1 = obj.GetArray();
	for (auto i{ 0 }; i < std::get<size_t>(result1); i++)
	{
		std::get<int*>(result1)[i] = 5 * i;
	}

	LegacyFoo5 obj2(5);
	obj2 = std::move(obj);
	auto [ptr, size_] = obj.GetArray();
	EXPECT_EQ(nullptr, ptr);
	EXPECT_EQ(0, size_);

	auto result2 = obj2.GetArray();

	for (auto i{ 0 }; i < std::get<size_t>(result2); i++)
	{
		EXPECT_EQ(5 * i, std::get<int*>(result2)[i]);
	}
}