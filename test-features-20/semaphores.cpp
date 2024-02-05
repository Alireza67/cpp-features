#include "pch.h"
#include <semaphore>

TEST(semaphore, binary)
{
	std::binary_semaphore testSemaphore{1};
	EXPECT_EQ(1, testSemaphore.max());
	EXPECT_TRUE(testSemaphore.try_acquire());
	EXPECT_FALSE(testSemaphore.try_acquire());
	testSemaphore.release();
	EXPECT_TRUE(testSemaphore.try_acquire());
}

TEST(semaphore, counting)
{
	std::counting_semaphore<2> testSemaphore{ 1 };
	EXPECT_EQ(2, testSemaphore.max());
	EXPECT_TRUE(testSemaphore.try_acquire());
	EXPECT_FALSE(testSemaphore.try_acquire());
	testSemaphore.release();
	testSemaphore.release();
	EXPECT_TRUE(testSemaphore.try_acquire());
	EXPECT_TRUE(testSemaphore.try_acquire());
}
