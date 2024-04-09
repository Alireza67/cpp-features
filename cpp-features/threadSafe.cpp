#include "pch.h"
#include <stack>
#include <mutex>
#include <thread>

template<typename T>
class ThreadSafeStack
{
public:
	ThreadSafeStack() {};
	ThreadSafeStack(const ThreadSafeStack& other)
	{
		std::lock_guard<std::mutex> lk(other.lock_);
		this->data_ = other.data_;
	}

	ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;

	void Push(T value)
	{
		std::lock_guard<std::mutex> lk(lock_);
		data_.push(std::move(value));
	}

	void Pop(T& value)
	{
		std::lock_guard<std::mutex> lk(lock_);

		if (data_.empty())
		{
			throw std::logic_error("There is no more data!");
		}

		value = std::move(data_.top());
		data_.pop();
	}

	std::shared_ptr<T> Pop()
	{
		std::lock_guard<std::mutex> lk(lock_);

		if (data_.empty())
		{
			throw std::logic_error("There is no more data!");
		}

		auto output = std::make_shared<T>(std::move(data_.top()));
		data_.pop();
		return output;
	}

	bool IsEmpyt()
	{
		std::lock_guard<std::mutex> lk(lock_);
		return data_.empty();
	}

	size_t Size()
	{
		std::lock_guard<std::mutex> lk(lock_);
		return data_.size();
	}

private:
	mutable std::mutex lock_;
	std::stack<T> data_;
};

void Producer(ThreadSafeStack<int>& stack, size_t counter)
{
	for (std::size_t i{ 0 }; i < counter; ++i)
	{
		stack.Push(i);
	}
}

TEST(ThreadSafe, stack_check_for_no_blocking)
{
	ThreadSafeStack<int> stack;
	size_t target = 30'000;
	auto t1 = std::thread(Producer, std::ref(stack), 10'000);
	auto t2 = std::thread(Producer, std::ref(stack), 10'000);
	auto t3 = std::thread(Producer, std::ref(stack), 10'000);
	t1.join();
	t2.join();
	t3.join();
	EXPECT_EQ(target, stack.Size());
}
