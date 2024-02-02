#include "pch.h"
#include <fstream>
using namespace std::literals;

TEST(io, linking_streams_use_tie) {
	std::ofstream outFile{ "fileTest.txt" };
	std::ifstream inFile{ "fileTest.txt" };
	inFile.tie(&outFile);
	outFile << "Hello!";
	std::string nextToken;
	inFile >> nextToken;
	EXPECT_EQ("Hello!"s, nextToken);
}

TEST(io, linking_streams_without_tie) {
	std::ofstream outFile{ "fileTest.txt" };
	std::ifstream inFile{ "fileTest.txt" };
	outFile << "Hello!";
	std::string nextToken;
	inFile >> nextToken;
	EXPECT_FALSE(nextToken.size());
}