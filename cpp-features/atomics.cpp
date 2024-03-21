#include "pch.h"
#include <atomic>
#include <mutex>
#include <future>

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

TEST(atomics, boolain)
{
	std::atomic<bool> flag{ true };
	EXPECT_FALSE(std::is_copy_constructible<decltype(flag)>::value);
	EXPECT_FALSE(std::is_copy_assignable<decltype(flag)>::value);
}

/*
	compare_exchange_weak() is one of compare-exchange primitives provided in C++11.
	It's weak in the sense that it returns false even if the value of the object is 
	equal to expected. This is due to spurious failure on some platforms where a 
	sequence of instructions (instead of one as on x86) are used to implement it. 
	On such platforms, context switch, reloading of the same address (or cache line) 
	by another thread, etc can fail the primitive. It's spurious as it's not the value
	of the object (not equal to expected) that fails the operation. 
	Instead, it's kind of timing issues.
*/

template<typename T>
void ChangeValue(std::atomic<T>& data, T value)
{
	data = value;
}

TEST(atomics, compare_exchange_weak)
{
	{
		std::atomic<bool> data{ false };
		auto expected = data.load();

		//Do some caluculation based on old data (expected)
		auto calcResult = true;

		auto result = data.compare_exchange_weak(expected, calcResult);
		EXPECT_TRUE(result);
		EXPECT_FALSE(expected);
		EXPECT_TRUE(data.load());
	}
	{
		std::atomic<bool> data{ false };
		auto expected = data.load();
		
		// Other Thread Change Value of data
		auto future = std::async(
			std::launch::async, ChangeValue<bool>, std::ref(data), true);
		future.get();

		//Do some caluculation based on old data (expected)
		auto calcResult = true;

		auto result = data.compare_exchange_weak(expected, calcResult);
		EXPECT_FALSE(result);
		EXPECT_TRUE(expected);
		EXPECT_TRUE(data.load());
	}
	{
		std::atomic<int> data{ 2 };
		auto expected = data.load();

		//Do some caluculation based on old data (expected)
		auto calcResult = std::pow(expected, 2);

		auto result = data.compare_exchange_weak(expected, calcResult);
		EXPECT_TRUE(result);
		EXPECT_EQ(2, expected);
		EXPECT_EQ(calcResult, data.load());
	}
	{
		std::atomic<int> data{ 50 };
		auto expected = data.load();

		// Other Thread Change Value of data
		auto newValue = 65;
		auto future = std::async(
			std::launch::async, ChangeValue<int>, std::ref(data), newValue);
		future.get();

		//Do some caluculation based on old data (expected)
		auto calcResult = std::pow(expected, 2);

		auto result = data.compare_exchange_weak(expected, calcResult);
		EXPECT_FALSE(result);
		EXPECT_EQ(newValue, expected);
		EXPECT_EQ(newValue, data.load());
	}
}

class ComplicatedData
{
public:
	ComplicatedData(int32_t p1, double p2, int64_t p3)
		:parameter1_(p1), parameter2_(p2), parameter3_(p3)
	{}

	ComplicatedData& operator=(const ComplicatedData& other)
	{
		parameter1_ = other.parameter1_;
		parameter2_ = other.parameter2_;
		parameter3_ = other.parameter3_;
		return *this;
	}

	/*
	
	'atomic<T> requires T to be trivially copyable, copy constructible, 
	move constructible, copy assignable, and move assignable.'
	
	ComplicatedData operator*(int coff)
	{
		auto tmp{*this};
		tmp.parameter1_ *= coff;
		tmp.parameter2_ *= coff;
		tmp.parameter3_ *= coff;
		return tmp;
	}
	*/

	auto operator<=>(const ComplicatedData& other) const = default;

private:
	int32_t parameter1_{};
	double parameter2_{};
	int64_t parameter3_{};
};

TEST(atomics, compare_exchange_weak_complicatedData)
{
	std::atomic<ComplicatedData> data({ 25, 32.5, 19000000000 });
	auto expected = data.load();

	// Other Thread Change Value of data
	auto newValue = ComplicatedData{ 52, -96.6564, 987654321 };
	auto future = std::async(
		std::launch::async, ChangeValue<ComplicatedData>, std::ref(data), newValue);
	future.get();

	//Do some caluculation based on old data (expected)
	auto calcResult = expected * 5;

	auto result = data.compare_exchange_weak(expected, calcResult);
	EXPECT_FALSE(result);
	EXPECT_EQ(newValue, expected);
	EXPECT_EQ(newValue, data.load());
}