#include "pch.h"
#include <stack>
#include <mutex>
#include <queue>
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

template<typename T>
void Producer(T& stack, size_t counter)
{
	for (std::size_t i{ 0 }; i < counter; ++i)
	{
		stack.Push(i);
	}
}

template<typename T>
void Consumer(T& stack, size_t counter)
{
	for (std::size_t i{ 0 }; i < counter; ++i)
	{
		stack.Pop();
	}
}

TEST(ThreadSafe, stack_check_for_no_blocking)
{
	ThreadSafeStack<int> stack;
	size_t target = 30'000;
	auto t1 = std::thread(Producer<ThreadSafeStack<int>>, std::ref(stack), 10'000);
	auto t2 = std::thread(Producer<ThreadSafeStack<int>>, std::ref(stack), 10'000);
	auto t3 = std::thread(Producer<ThreadSafeStack<int>>, std::ref(stack), 10'000);
	t1.join();
	t2.join();
	t3.join();
	EXPECT_EQ(target, stack.Size());
}

template<typename T>
class LockFreeStack
{
public:
	std::shared_ptr<T> Pop()
	{
		++threads_in_pop;
		node* old_head = head.load();
		while (old_head &&
			!head.compare_exchange_weak(old_head, old_head->next));
		std::shared_ptr<T> res;
		if (old_head)
		{
			res.swap(old_head->data);
		}
		try_reclaim(old_head);
		return res;
	}

	void Push(T const& data)
	{
		node* const new_node = new node(data);
		new_node->next = head.load();
		while (!head.compare_exchange_weak(new_node->next, new_node));
	}

private:
	struct node
	{
		std::shared_ptr<T> data;
		node* next;
		node(T const& data_) :
			data(std::make_shared<T>(data_))
		{}
	};

	std::atomic<node*> head;
	std::atomic<node*> to_be_deleted{};
	std::atomic<unsigned> threads_in_pop;

	static void delete_nodes(node* nodes)
	{
		while (nodes)
		{
			node* next = nodes->next;
			delete nodes;
			nodes = next;
		}
	}
	
	void try_reclaim(node* old_head)
	{
		if (threads_in_pop == 1)
		{
			node* nodes_to_delete = to_be_deleted.exchange(nullptr);
			if (!--threads_in_pop)
			{
				delete_nodes(nodes_to_delete);
			}
			else if (nodes_to_delete)
			{
				chain_pending_nodes(nodes_to_delete);
			}
			delete old_head;
		}
		else
		{
			chain_pending_node(old_head);
			--threads_in_pop;
		}
	}
	
	void chain_pending_nodes(node* nodes)
	{
		node* last = nodes;
		while (node* const next = last->next)
		{
			last = next;
		}
		chain_pending_nodes(nodes, last);
	}
	
	void chain_pending_nodes(node* first, node* last)
	{
		if(last)
		{
			last->next = to_be_deleted;
			while (!to_be_deleted.compare_exchange_weak(
				last->next, first));
		}
	}
	
	void chain_pending_node(node* n)
	{
		chain_pending_nodes(n, n);
	}
};

TEST(LockFree, lock_free_stack)
{
	LockFreeStack<int> stack;
	auto t1 = std::thread(Producer<LockFreeStack<int>>, std::ref(stack), 100'000);
	auto t2 = std::thread(Consumer<LockFreeStack<int>>, std::ref(stack), 25'000);
	auto t3 = std::thread(Consumer<LockFreeStack<int>>, std::ref(stack), 25'000);
	auto t4 = std::thread(Consumer<LockFreeStack<int>>, std::ref(stack), 25'000);
	auto t5 = std::thread(Consumer<LockFreeStack<int>>, std::ref(stack), 25'000);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
}

template<typename T>
class ThreadSafeQueue
{
public:
	ThreadSafeQueue(){}

	void Push(T new_value)
	{
		{
			std::lock_guard<std::mutex> lk(lock_);
			data_.push(std::move(new_value));
		}
		notifyPoint_.notify_one();
	}

	void Wait_And_Pop(T& value)
	{
		std::unique_lock<std::mutex> lk(lock_);
		notifyPoint_.wait(lk, [this] {return !data_.empty(); });
		value = std::move(data_.front());
		data_.pop();
	}

	std::shared_ptr<T> Wait_And_Pop()
	{
		std::unique_lock<std::mutex> lk(lock_);
		notifyPoint_.wait(lk, [this] {return !data_.empty(); });
		std::shared_ptr<T> res(std::make_shared<T>(std::move(data_.front())));
		data_.pop();
		return res;
	}

	bool Try_Pop(T& value)
	{
		std::lock_guard<std::mutex> lk(lock_);
		if (data_.empty())
		{
			return false;
		}
		value = std::move(data_.front());
		data_.pop();
		return true;
	}

	std::shared_ptr<T> Try_Pop()
	{
		std::lock_guard<std::mutex> lk(lock_);
		if (data_.empty())
		{
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> res(std::make_shared<T>(std::move(data_.front())));
		data_.pop();
		return res;
	}

	bool Empty() const
	{
		std::lock_guard<std::mutex> lk(lock_);
		return data_.empty();
	}

private:
	std::queue<T> data_;
	mutable std::mutex lock_;
	std::condition_variable notifyPoint_;
};
