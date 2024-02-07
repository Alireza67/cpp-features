#include "pch.h"
#include <thread>
#include <latch>
#include <barrier>
#include <array>

using namespace std::literals;

TEST(threads, latch)
{
    constexpr std::uint8_t max_threads = 5U;
    std::array<std::jthread, max_threads> works{};

    std::latch work_latch(std::size(works));
    std::latch exit_latch(1U);

    for (auto it = std::begin(works); it != std::end(works); ++it)
    {
        *it = std::jthread([&work_latch, &exit_latch]() noexcept 
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

TEST(threads, barrier)
{
    constexpr std::uint8_t max_threads = 50;
    std::array<std::jthread, max_threads> works{};
    std::atomic<unsigned int> counter{};

    std::barrier work_barrier(std::size(works), [&counter]() noexcept {
        EXPECT_TRUE(counter >= max_threads);
        EXPECT_FALSE(counter % max_threads);
        });

    for (auto it = std::begin(works); it != std::end(works); ++it)
    {
        *it = std::jthread([&work_barrier, &counter]() noexcept {
                counter++;
                work_barrier.arrive_and_wait();
                counter++;
                work_barrier.arrive_and_wait();
                counter++;
                work_barrier.arrive_and_wait();
                counter++;
                work_barrier.arrive_and_wait();
                });
    }
}