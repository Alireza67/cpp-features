#include "pch.h"
#include <source_location>
#include <stdexcept>
#include <string>
#include <filesystem>

using namespace std::literals;
namespace fs = std::filesystem;

class TestExceptin : public std::exception
{
public:
	explicit TestExceptin(
		std::string msg, 
		std::source_location location = std::source_location::current())
		:msg_(std::move(msg)), location_(std::move(location))
	{
	}

	char const* what() const override
	{
		return msg_.c_str();
	}

	const std::source_location& where() const
	{
		return location_;
	}

private:
	std::string msg_{};
	std::source_location location_{};
};

void Foo()
{
	throw TestExceptin("Test exception!");
}

TEST(sourceLocation, checkByException)
{
	try
	{
		Foo();
	}
	catch (const TestExceptin& e)
	{		
		EXPECT_EQ("Test exception!"s, std::string(e.what()));
		EXPECT_EQ("sourceLocation.cpp"s, fs::path(std::string(e.where().file_name())).filename().string());
		EXPECT_EQ("Foo"s, std::string(e.where().function_name()));
		EXPECT_EQ(37u, e.where().line());
	}
}
