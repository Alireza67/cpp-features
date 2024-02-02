#include "pch.h"

class Foo
{
public:
	Foo(const double& value)
	{
		age = value;
	}

	double GetAge()
	{
		return age;
	}

	Foo& operator++()
	{
		age++;
		return *this;
	}

	Foo& operator++(int)
	{
		auto tmp{*this};
		operator++();
		return tmp;
	}

private:
	double age{};
};

template<typename T>
concept increment = requires(T x) 
{ 
	x++; ++x;
	x.GetAge();
};

template<increment T, typename U>
U GetFinalValue(T& input)
{
	input++;
	++input;
	return input.GetAge();
}

TEST(Concept, overload)
{
	Foo obj(1);

	auto obj2 = ++obj;
	EXPECT_EQ(2, obj.GetAge());
	EXPECT_EQ(2, obj2.GetAge());

	auto obj3 = obj++;
	EXPECT_EQ(3, obj.GetAge());
	EXPECT_EQ(2, obj3.GetAge());

	auto result = GetFinalValue<Foo, int>(obj);
	EXPECT_EQ(5, result);
}


template<typename T>
concept wired = std::integral<T> && sizeof(T) == 4;

template<wired T>
T Printer(T input)
{
	return input;
}

TEST(Concept, integralCheck)
{
	EXPECT_EQ(5, Printer(static_cast<int>(5)));
	//EXPECT_EQ(5, Printer(static_cast<short>(5))); couldn't compile
	//EXPECT_EQ(5, Printer(static_cast<float>(5))); couldn't compile
}
