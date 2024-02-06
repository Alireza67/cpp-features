#include "pch.h"
#include <thread>
#include <latch>
#include <array>

using namespace std::literals;

TEST(latch, simple)
{
    constexpr std::uint8_t max_threads = 5U;
    std::array<std::jthread, max_threads> works{};

    std::latch work_latch(std::size(works));
    std::latch exit_latch(1U);

    for (auto it = std::begin(works); it != std::end(works); ++it)
    {
        *it = std::jthread([&work_latch, &exit_latch, index = std::distance(std::begin(works), it)]() noexcept 
            {
                std::this_thread::sleep_for(1s);
                work_latch.count_down();
                exit_latch.wait();
            });
    }

    //blocks until the counter reaches zero
    work_latch.wait();

    for (auto& item : works)
    {
        EXPECT_TRUE(item.joinable());
    }

    exit_latch.count_down();
    std::this_thread::sleep_for(100ms);
    for (auto& item : works)
    {
        item.join();
        EXPECT_FALSE(item.joinable());
    }
}