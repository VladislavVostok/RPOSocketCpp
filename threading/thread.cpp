#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

int main()
{
    unsigned long long g_count = 0;

    std::mutex g_count_mutex;

    std::thread t1([&]()
        {
            for (auto i = 0; i < 1'000'000; ++i) {
                g_count_mutex.lock();
                g_count += 1;
                g_count_mutex.unlock();
            }
        });

    std::thread t2([&]()
        {
            for (auto i = 0; i < 1'000'000; ++i) {
                g_count_mutex.lock();
                g_count += 1;
                g_count_mutex.unlock();
            }
        });


    t1.join();
    t2.join();

    std::cout << g_count;

    return 0;
}