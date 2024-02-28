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
