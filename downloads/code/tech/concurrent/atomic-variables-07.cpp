#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> counter(0);

void conditional_increment() {
    for (int i = 0; i < 1000; ++i) {
        int expected = counter.load();
        while (expected < 500 && !counter.compare_exchange_weak(expected, expected + 1)) {
            // 重试，直到成功或条件不满足
        }
    }
}

int main() {
    std::thread t1(conditional_increment);
    std::thread t2(conditional_increment);

    t1.join();
    t2.join();

    std::cout << "Final counter value: " << counter << std::endl;
    return 0;
}