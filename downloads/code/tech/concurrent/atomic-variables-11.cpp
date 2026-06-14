#include <atomic>
#include <iostream>
#include <thread>

struct StampedValue {
    int value;
    int stamp;
};

std::atomic<StampedValue> atomicStampedValue{StampedValue{0, 0}};

void increment() {
    for (int i = 0; i < 1000; ++i) {
        StampedValue expected = atomicStampedValue.load();
        StampedValue newValue;
        do {
            newValue = {expected.value + 1, expected.stamp + 1};
        } while (!atomicStampedValue.compare_exchange_weak(expected, newValue));
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    StampedValue finalValue = atomicStampedValue.load();
    std::cout << "Final value: " << finalValue.value << ", Final stamp: " << finalValue.stamp << std::endl;
    return 0;
}