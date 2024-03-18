#include "pch.h"
#include <atomic>
#include <mutex>

class SpinLock
{
public:
	void lock()
	{
		/*
		* The std::memory_order_acquire ensures that all memory operations 
		* that happen before the test_and_set in the modification order are 
		* visible to the current thread before it acquires the lock.
		* It ensures that no memory operations that happen after 
		* the test_and_set in the modification order can be reordered 
		* to happen before it. 
		*/
		while (flag_.test_and_set(std::memory_order_acquire));
	}

	void unlock()
	{
		/*
		* The std::memory_order_release ensures that all memory operations 
		* that happen after the clear in the modification order are 
		* visible to other threads after they acquire the lock.
		* It ensures that no memory operations that happen before 
		* the clear in the modification order can be reordered
		* to happen after it. 
		*/
		flag_.clear(std::memory_order_release);
	}

private:
	std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};


SpinLock testLock;
std::size_t counter{};

void IncreaseValue(size_t number)
{
	for (auto i{ 0 }; i < number; i++)
	{
		std::lock_guard<SpinLock> lk(testLock);
		counter++;
	}
}

TEST(atomics, spinloack)
{
	std::vector<std::jthread> vectorList{};
	auto threadNumber = std::thread::hardware_concurrency();
	size_t steps{ 100'000 };
	auto target{ steps * threadNumber };

	for (auto i{ 0 }; i < threadNumber; ++i)
	{
		vectorList.emplace_back(IncreaseValue, steps);
	}

	for (auto& item : vectorList)
	{
		item.join();
	}

	EXPECT_EQ(target, counter);
}

std::atomic<int> z;
std::atomic<bool> x, y;

void Write_x_then_y()
{
	x.store(true, std::memory_order_relaxed);
	y.store(true, std::memory_order_relaxed);
}

void Read_y_then_x()
{
	while (!y.load(std::memory_order_consume));
	if (x.load(std::memory_order_relaxed))
	{
		++z;
	}
}

TEST(atomics, memory_order_relaxed)
{
	x = false;
	y = false;
	z = 0;

	std::thread a(Write_x_then_y);
	std::thread b(Read_y_then_x);
	b.join();
	a.join();
	EXPECT_TRUE(z.load() != 0);
}

std::atomic<std::string*> ptr{};
int data{};
size_t counterT{};

void producer()
{
	std::string* p = new std::string("Hello");
	data = 42;
	ptr.store(p, std::memory_order_release);
}

void consumer()
{
	std::string* p2;
	while (!(p2 = ptr.load(std::memory_order_consume)));
	assert(*p2 == "Hello"); // never fires
	assert(data == 42);
}

TEST(atomics, memory_order)
{
	std::thread t1(producer);
	std::thread t2(consumer);
	t1.join(); t2.join();
}

