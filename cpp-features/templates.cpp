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

struct Department
{
	int departmentCode;
	std::size_t numberOfEmployee;
	std::string name;
};

template<Department& part>
class Employee
{
public:
	std::string Print()
	{
		return part.name + " Department has "s + std::to_string(part.numberOfEmployee)
			+ " employees and it's code is "s + std::to_string(part.departmentCode);
	}
};

TEST(NonType, reference)
{
	static Department math{ -5, 250, "math"s};
	Employee<math> obj;
	EXPECT_EQ("math Department has 250 employees and it's code is -5"s, obj.Print());
}
