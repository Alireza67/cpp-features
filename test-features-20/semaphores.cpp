#include "pch.h"
#include <semaphore>
#include <stop_token>
#include <thread>
#include <queue>
#include <mutex>
#include <future>

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

template<typename T>
class QueueWithSemaphore
{
public:
	QueueWithSemaphore(T initValue)
		:semaphore_(initValue) {}

	void Push(T& element)
	{
		{
			std::lock_guard<std::mutex> lk(lock_);
			queue_.push(element);
		}
		semaphore_.release();
	}

	T Pop()
	{
		semaphore_.acquire();
		std::lock_guard<std::mutex> lk(lock_);
		auto tmp = std::move(queue_.front());
		queue_.pop();
		return tmp;
	}

private:
	std::mutex lock_;
	std::queue<T> queue_{};
	std::counting_semaphore<> semaphore_;
};

template<typename T>
void Generator(QueueWithSemaphore<T>& queue, T data)
{
	queue.Push(data);
}

template<typename T>
T Reader(QueueWithSemaphore<T>& queue)
{
	return queue.Pop();
}

TEST(semaphore, queue_with_semaphore)
{
	auto target{ 10 };
	QueueWithSemaphore<int> queue(0);
	auto generator = std::async(std::launch::deferred, Generator<int>, std::ref(queue), 10);
	auto reader = std::async(std::launch::async, Reader<int>, std::ref(queue));
	auto result = reader.wait_for(100ms);
	EXPECT_EQ(std::future_status::timeout, result);
	result = reader.wait_for(100ms);
	EXPECT_EQ(std::future_status::timeout, result);
	generator.get();
	result = reader.wait_for(100ms);
	EXPECT_EQ(std::future_status::ready, result);
	EXPECT_EQ(target, reader.get());
}

