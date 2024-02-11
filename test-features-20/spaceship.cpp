#include "pch.h"


class Soo
{
public:
	explicit Soo(double length, int age, std::string version)
		:length_(length), age_(age), version_(std::move(version))
	{ 
	}

	auto operator<=>(const Soo & other) const = default;

private:
	double length_{};
	int age_{};
	std::string version_{};
};

TEST(spaceship, simple)
{
	Soo obj(4.698, 80, "6.3.10");
	Soo other(4.697, 80, "6.3.9");

	EXPECT_FALSE(obj == other);
	EXPECT_FALSE(obj <= other);
	EXPECT_FALSE(obj < other);
	EXPECT_TRUE(obj != other);
	EXPECT_TRUE(obj >= other);
	EXPECT_TRUE(obj > other);
}

class NewSoo
{
public:
	explicit NewSoo(double length, int age, std::string version)
		:length_(length), age_(age), version_(std::move(version))
	{
	}

	auto operator<=>(const NewSoo& other) const = default;

private:
	std::string version_{};
	double length_{};
	int age_{};
};

TEST(spaceship, new_order)
{
	NewSoo obj(4.698, 80, "6.3.10");
	NewSoo other(4.697, 80, "6.3.9");

	EXPECT_FALSE(obj == other);
	EXPECT_TRUE(obj <= other);
	EXPECT_TRUE(obj < other);
	EXPECT_TRUE(obj != other);
	EXPECT_FALSE(obj >= other);
	EXPECT_FALSE(obj > other);
}
