#include "pch.h"
#include <memory>

TEST(smartPointers, weakptr_reference_count)
{
	auto target{ 10 };
	auto obj1 = std::make_shared<int>(target);
	EXPECT_EQ(target, *obj1);
	std::weak_ptr<int> weakObj = obj1;
	EXPECT_EQ(1, obj1.use_count());
}

TEST(smartPointers, weakptr_lock_after_free)
{
	auto target{ 10 };
	auto obj1 = std::make_shared<int>(target);
	std::weak_ptr<int> weakObj = obj1;
	obj1.reset();
	EXPECT_EQ(nullptr, weakObj.lock());
}

TEST(smartPointers, weakptr_lock)
{
	auto target{ 10 };
	auto obj1 = std::make_shared<int>(target);
	std::weak_ptr<int> weakObj = obj1;
	auto obj2 = weakObj.lock();
	EXPECT_EQ(*obj1, *obj2);
	EXPECT_EQ(2, obj1.use_count());
	obj1.reset();
	EXPECT_EQ(nullptr, obj1);
	EXPECT_EQ(target, *obj2);
}