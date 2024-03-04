#include "pch.h"
#include <thread>
#include <memory>
#include <mutex>
#include <future>

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


void Counter(size_t number, std::atomic<size_t>& output)
{
	for (int i{}; i < number; i++)
	{
		output++;
	}
}

auto GetTime = []() {return std::chrono::high_resolution_clock::now(); };
TEST(threads, check_hardware)
{
	size_t target = 1'000'000;
	auto numberOfHardwareThread { std::thread::hardware_concurrency()};


	std::atomic<size_t> result2 = 0;
	std::vector<std::thread> threadList2{};
	auto start2 = GetTime();
	for (auto i{ 0 }; i < 400; i++)
	{
		threadList2.emplace_back(Counter, (target / 400), std::ref(result2));
	}
	for (auto& item : threadList2)
	{
		item.join();
	}
	auto stop2 = GetTime();
	EXPECT_EQ(target, result2);
	std::chrono::duration<double> elapsed2 = stop2 - start2;


	std::atomic<size_t> result = 0;
	std::vector<std::thread> threadList{};
	auto start = GetTime();
	for (auto i{ 0 }; i < numberOfHardwareThread; i++)
	{
		threadList.emplace_back(Counter, (target / numberOfHardwareThread), std::ref(result));
	}
	for (auto& item : threadList)
	{
		item.join();
	}
	auto stop = GetTime();
	EXPECT_EQ(target, result);
	std::chrono::duration<double> elapsed = stop - start;


	EXPECT_LT(elapsed.count(), elapsed2.count());
}

TEST(threads, threadId)
{
	std::thread t0;
	auto tmp = t0.get_id();
	auto id0 = *reinterpret_cast<unsigned int*>(&tmp);
	EXPECT_EQ(0, id0);

	auto id{ 2 };
	auto data{ 1.0 };
	auto t = std::thread(Dummy, id, std::ref(data));
	tmp = t.get_id();
	auto id1 = *reinterpret_cast<unsigned int*>(&tmp);
	EXPECT_TRUE(id1);
	t.join();
	tmp = t.get_id(); 
	id1 = *reinterpret_cast<unsigned int*>(&tmp);
	EXPECT_EQ(0, id1);
}

class Sdata
{
public:
	Sdata(std::vector<int> data)
		:data_(std::move(data)) {}

	std::vector<int> GetData() { return data_; }

	friend void DeadlockSwap(Sdata& lhs, Sdata& rhs);
	friend void SafelockSwap(Sdata& lhs, Sdata& rhs);
	friend void SafelockSwapDefer(Sdata& lhs, Sdata& rhs);

private:
	std::vector<int> data_{};
	std::mutex lk_{};
};

void DeadlockSwap(Sdata& lhs, Sdata& rhs)
{
	if (&lhs == &rhs)
	{
		return;
	}

	std::lock_guard<std::mutex> lk(lhs.lk_);
	std::lock_guard<std::mutex> lk2(rhs.lk_);
	std::swap(lhs.data_, rhs.data_);
}

void SafelockSwap(Sdata& lhs, Sdata& rhs)
{
	if (&lhs == &rhs)
	{
		return;
	}

	std::scoped_lock lk(lhs.lk_, rhs.lk_);
	std::swap(lhs.data_, rhs.data_);
}

void SafelockSwapDefer(Sdata& lhs, Sdata& rhs)
{
	if (&lhs == &rhs)
	{
		return;
	}

	std::unique_lock lk1(lhs.lk_, std::defer_lock);
	std::unique_lock lk2(rhs.lk_, std::defer_lock);
	std::lock(lk1, lk2);
	std::swap(lhs.data_, rhs.data_);
}

void SwapOddTimes(Sdata& lhs, Sdata& rhs, void(*swapper)(Sdata& lhs, Sdata& rhs))
{
	for (int i{}; i < 10001; i++)
	{
		swapper(lhs, rhs);
	}
}

void DeadLockScenario(Sdata& lhs, Sdata& rhs)
{
	auto t1 = std::thread(SwapOddTimes, std::ref(lhs), std::ref(rhs), &DeadlockSwap);
	auto t2 = std::thread(SwapOddTimes, std::ref(rhs), std::ref(lhs), &DeadlockSwap);
	t1.join();
	t2.join();
}

void SafeLockScenario(Sdata& lhs, Sdata& rhs)
{
	auto t1 = std::thread(SwapOddTimes, std::ref(lhs), std::ref(rhs), &SafelockSwap);
	auto t2 = std::thread(SwapOddTimes, std::ref(rhs), std::ref(lhs), &SafelockSwap);
	t1.join();
	t2.join();
}

void SafeLockScenarioDefer(Sdata& lhs, Sdata& rhs)
{
	auto t1 = std::thread(SwapOddTimes, std::ref(lhs), std::ref(rhs), &SafelockSwapDefer);
	auto t2 = std::thread(SwapOddTimes, std::ref(rhs), std::ref(lhs), &SafelockSwapDefer);
	t1.join();
	t2.join();
}

void DetectStuck(Sdata& lhs, Sdata& rhs)
{
	auto asyncFuture = std::async(std::launch::async, DeadLockScenario, std::ref(lhs), std::ref(rhs));
	if (asyncFuture.wait_for(std::chrono::seconds(3)) == std::future_status::timeout)
	{
		exit(1);
	}
}

TEST(threads, deadlockSwap)
{
	auto target1 = std::vector{ 1,2,3,4,5 };
	auto target2 = std::vector{ 10,20,30,40,50 };
	Sdata a(target1);
	Sdata b(target2);

	auto t = std::thread(SwapOddTimes, std::ref(a), std::ref(b), &DeadlockSwap);
	t.join();
	EXPECT_EQ(target2, a.GetData());
	EXPECT_DEATH(DetectStuck(a, b),".*");	
}

TEST(threads, safeSwap)
{
	auto target1 = std::vector{ 1,2,3,4,5 };
	auto target2 = std::vector{ 10,20,30,40,50 };
	Sdata a(target1);
	Sdata b(target2);
	SafeLockScenario(a, b);
	EXPECT_EQ(target1, a.GetData());
	EXPECT_EQ(target2, b.GetData());
}

TEST(threads, safeSwap_deferlock)
{
	auto target1 = std::vector{ 1,2,3,4,5 };
	auto target2 = std::vector{ 10,20,30,40,50 };
	Sdata a(target1);
	Sdata b(target2);
	SafeLockScenarioDefer(a, b);
	EXPECT_EQ(target1, a.GetData());
	EXPECT_EQ(target2, b.GetData());
}

class ThreadLocalSample
{
public:
	int GetId()
	{
		return id_;
	}

	void SetId(int id)
	{
		id_ = id;
	}
private:
	static thread_local int id_;
};
thread_local int ThreadLocalSample::id_ = 10;

void AnotherMethod(ThreadLocalSample& obj)
{
	EXPECT_EQ(10, obj.GetId());
	obj.SetId(8);
	EXPECT_EQ(8, obj.GetId());
}

TEST(threads, threadlocal)
{
	ThreadLocalSample obj;
	EXPECT_EQ(10, obj.GetId());
	obj.SetId(6);
	EXPECT_EQ(6, obj.GetId());
	
	auto t = std::thread(AnotherMethod, std::ref(obj));
	t.join();
	EXPECT_EQ(6, obj.GetId());
}

class AudioCard
{
public:
	explicit AudioCard() = default;
	virtual ~AudioCard() = default;

	virtual void Initialize() = 0;
};

class MockAudioCard : public AudioCard
{
public:
	MOCK_METHOD(void, Initialize, (), (override));
};

class AudioCardDriver
{
public:
	explicit AudioCardDriver(std::shared_ptr<AudioCard> audioCard)
		:audioCard_(std::move(audioCard)) {}

	void Run()
	{
		std::call_once(init_, &AudioCard::Initialize, audioCard_.get());
	}

	void RunWithoutRestriction()
	{
		audioCard_->Initialize();
	}

private:
	std::once_flag init_;
	std::shared_ptr<AudioCard> audioCard_{};
};

TEST(threads, call_once)
{
	auto audioCard = std::make_shared<MockAudioCard>();
	EXPECT_CALL(*audioCard, Initialize()).Times(1);
	AudioCardDriver driver(std::dynamic_pointer_cast<AudioCard>(audioCard));

	auto t1 = std::thread(&AudioCardDriver::Run, &driver);
	auto t2 = std::thread(&AudioCardDriver::Run, &driver);
	auto t3 = std::thread(&AudioCardDriver::Run, &driver);
	t1.join();
	t2.join();
	t3.join();
}

TEST(threads, without_call_once)
{
	auto audioCard = std::make_shared<MockAudioCard>();
	EXPECT_CALL(*audioCard, Initialize()).Times(3);
	AudioCardDriver driver(std::dynamic_pointer_cast<AudioCard>(audioCard));

	auto t1 = std::thread(&AudioCardDriver::RunWithoutRestriction, &driver);
	auto t2 = std::thread(&AudioCardDriver::RunWithoutRestriction, &driver);
	auto t3 = std::thread(&AudioCardDriver::RunWithoutRestriction, &driver);
	t1.join();
	t2.join();
	t3.join();
}

std::string GenerateGreetingMessage(std::string name, int age)
{
	return "Hello Dear "s + name + " with "s + std::to_string(age) + " years old!"s;
}

TEST(threads, packaged_task_)
{
	std::packaged_task<std::string(std::string, int)> task(GenerateGreetingMessage);
	EXPECT_TRUE(task.valid());
	auto f = task.get_future();
	EXPECT_FALSE(f._Is_ready());
	task("John", 40);
	auto result = f.get();
	EXPECT_EQ("Hello Dear John with 40 years old!"s, result);
}

void CalculationUnit(std::promise<int>& promise, int state)
{
	if (state)
	{
		promise.set_value(10);
	}
	else
	{
		promise.set_exception(std::make_exception_ptr(std::logic_error("SPECIFIC ERROR!")));
	}
}

void CheckAnswer(std::shared_future<int> future, int state)
{
	if (state)
	{
		EXPECT_EQ(10, future.get());
	}
	else
	{
		try
		{
			future.get();
		}
		catch (const std::exception& e)
		{
			auto msg = std::string(e.what());
			EXPECT_EQ("SPECIFIC ERROR!", msg);
		}
	}
}

TEST(threads, promise_setValue)
{
	std::promise<int> promise;
	auto future = promise.get_future().share();

	std::thread t1(CalculationUnit, std::ref(promise), 1);
	std::thread t2(CheckAnswer, future, 1);

	t1.join();
	t2.join();
}

TEST(threads, promise_setException)
{
	std::promise<int> promise;
	auto future = promise.get_future().share();

	std::thread t1(CalculationUnit, std::ref(promise), 0);
	std::thread t2(CheckAnswer, future, 0);

	t1.join();
	t2.join();
}

TEST(threads, shared_future)
{
	std::promise<int> promise;
	auto future = promise.get_future().share();

	std::thread t1(CalculationUnit, std::ref(promise), 1);
	std::thread t2(CheckAnswer, future, 1);
	std::thread t3(CheckAnswer, future, 1);

	t1.join();
	t2.join();
	t3.join();
}