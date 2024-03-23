#pragma once
#include "pch.h"
#include <fstream>
#include <iterator>

TEST(files, reading)
{
	auto fileName {"simpleTest.bin"s};
	auto target{ "this is a simple test!"s };
	{
		std::fstream file(fileName, std::ios::out | std::ios::binary);
		file.write(target.data(), target.size());
	}

	auto reader = [](const std::string& fileName) {
		std::ifstream file(fileName, std::ios::in | std::ios::binary);
		return std::string(
			std::istreambuf_iterator<char>{file}, 
			std::istreambuf_iterator<char>{});
		};

	auto result = reader(fileName);
	EXPECT_EQ(target, result);
}
