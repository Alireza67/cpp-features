#include "pch.h"
#include <deque>

TEST(containers, deque)
{
	std::deque<int> target{ 4,3,2,1,0,0,1,2,3,4 };
	std::deque<int> input;
	
	for (int i{}; i < 5; i++)
	{
		input.push_front(i);
		input.push_back(i);
	}

	EXPECT_EQ(target, input);
}