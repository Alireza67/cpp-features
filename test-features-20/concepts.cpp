#include "pch.h"

class Foo
{
public:
	Foo(const int& value)
	{
		age = value;
	}

	int GetAge()
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
	int age{};
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
