#include "pch.h"

class License
{
public:
	explicit License(uint8_t minimum, uint8_t maximum)
		:minumumAge_(minimum), maximumAge_(maximum)
	{}

	bool operator()(uint8_t age) const
	{
		if (age >= minumumAge_ && age <= maximumAge_)
		{
			return true;
		}
		return false;
	}

private:
	uint8_t minumumAge_{};
	uint8_t maximumAge_{};
};

TEST(functor, simple)
{
	License license{18, 60};
	EXPECT_FALSE(license(17));
	EXPECT_FALSE(license(61));
	EXPECT_TRUE(license(18));
	EXPECT_TRUE(license(60));
	EXPECT_TRUE(license(36));
}
