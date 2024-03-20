#include "pch.h"
#include <functional>


std::string PrintGreeting(std::string_view country, std::string_view name)
{
	return "Hello "s + name.data() + ". Welcome to " + country.data() + "!";
}

TEST(bind, bind_front)
{
	auto CustomPrint = std::bind_front(PrintGreeting, "Iran"s);
	EXPECT_EQ("Hello John. Welcome to Iran!", CustomPrint("John"));
}

TEST(getenv_, simple)
{
	auto env_var_name = "PATH";
	if (auto res = std::getenv(env_var_name);
		res != nullptr)
	{
		SUCCEED();
		return;
	}
	FAIL();
}
