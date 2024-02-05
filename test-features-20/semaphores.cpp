#include "pch.h"
#include <semaphore>
#include <stop_token>
#include <thread>

using namespace std::literals;
TEST(semaphore, binary)
{
	std::binary_semaphore testSemaphore{1};
	EXPECT_EQ(1, testSemaphore.max());
	EXPECT_TRUE(testSemaphore.try_acquire());
	EXPECT_FALSE(testSemaphore.try_acquire());
	testSemaphore.release();
	EXPECT_TRUE(testSemaphore.try_acquire());
}



std::binary_semaphore gloabSemaphore{ 1 };

void M1(std::stop_token stoken)
{
	gloabSemaphore.acquire();
	while (!stoken.stop_requested())
	{
		std::this_thread::sleep_for(10ms);
	}
}

TEST(semaphore, get_and_release_in_different_threads)
{
	auto t1 = std::jthread(&M1);
	std::this_thread::sleep_for(50ms);
	EXPECT_FALSE(gloabSemaphore.try_acquire());
	gloabSemaphore.release();
	EXPECT_TRUE(gloabSemaphore.try_acquire());
	t1.request_stop();
	t1.join();
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
