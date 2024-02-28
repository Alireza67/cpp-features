#include "pch.h"
#include <thread>
#include <memory>

class NewTask
{
public:
	void operator() ()
	{
		value_ = 10;
	}

	int GetValue()
	{ 
		return value_;
	}

private:
	int value_{};
};

TEST(threads, functor)
{
	NewTask obj;
	EXPECT_FALSE(obj.GetValue());

	auto t = std::thread(obj);
	t.join();
	EXPECT_EQ(0, obj.GetValue());

	t = std::thread(std::ref(obj));
	t.join();
	EXPECT_EQ(10, obj.GetValue());
}

void Dummy(int id, double& data)
{
	data = id * 2;
}

TEST(threads, referenceParameters)
{
	auto id{ 2 };
	auto data{ 1.0 };

	auto t = std::thread(Dummy, id, std::ref(data));
	t.join();

	EXPECT_EQ(4.0, data);
}


void DummyPtr(std::unique_ptr<double>&& ptr, double& output)
{
	output = *ptr;
}

TEST(threads, rvalueParameter_namedValue)
{
	double output{ };
	auto ptr = std::make_unique<double>(9);
	EXPECT_TRUE(ptr);

	auto t = std::thread(DummyPtr, std::move(ptr), std::ref(output));
	t.join();

	EXPECT_EQ(9.0, output);
	EXPECT_FALSE(ptr);
}

TEST(threads, rvalueParameter_temporary)
{
	double output{ };

	auto t = std::thread(DummyPtr, std::make_unique<double>(9), std::ref(output));
	t.join();

	EXPECT_EQ(9.0, output);
}

class My_jthread
{
public:
	My_jthread() = default;

	template<typename Callable, typename ... Args>
	explicit My_jthread(Callable&& function, Args&& ... args)
		:t_(std::forward<Callable>(function), std::forward<Args>(args)...)
	{}

	~My_jthread()
	{
		if (t_.joinable())
		{
			t_.join();
		}
	}

	My_jthread(const My_jthread& rhs) = delete;
	My_jthread& operator=(const My_jthread& rhs) = delete;

	My_jthread& operator=(My_jthread&& rhs) noexcept
	{
		if (&rhs == this)
		{
			return *this;
		}

		if (t_.joinable())
		{
			t_.join();
		}
		t_ = std::move(rhs.t_);
		return *this;
	}

	bool Joinable()
	{
		return t_.joinable();
	}

	void Join()
	{
		t_.join();
	}

private:
	std::thread t_;
};

TEST(threads, My_jthreadTest)
{
	double target{ 9.0 };
	double output{ };
	My_jthread obj;
	EXPECT_FALSE(obj.Joinable());
	{
		auto inner = My_jthread(DummyPtr, std::make_unique<double>(target), std::ref(output));
		obj = std::move(inner);
		EXPECT_FALSE(inner.Joinable());
	}
	EXPECT_TRUE(obj.Joinable());
	obj.Join();
	EXPECT_EQ(target, output);
}
