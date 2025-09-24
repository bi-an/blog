#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
using namespace std;

constexpr int run_times = 10;
constexpr uint32_t num_threads = 10;
constexpr uint64_t increments_per_thread = 1024 * 1024 * 32;
constexpr uint64_t max_count = increments_per_thread * num_threads;

atomic<uint64_t> counter;

/** Random Delay in microseonds */
std::random_device rd;
thread_local std::mt19937 gen(rd());
thread_local std::uniform_int_distribution<> dis(0, 4095);

void unpredictableDelay(int extra = 0) {
    if (dis(gen) == 0) {
        this_thread::sleep_for(chrono::nanoseconds(2000 + extra));
    }
}

/** thread function for counting */
void worker(int id) {
    for (int i = 0; i < increments_per_thread; ++i) {
        uint64_t old = counter.load(memory_order_relaxed);
        // 如果 load(relaxed) 不能看到当前最新值
        // 那么 CAS 就会加多次，最终结果会大于 max_count
        while (old < max_count &&
               !counter.compare_exchange_weak(old, old + 1, memory_order_relaxed)) {
            // old is updated with the current value of counter
            unpredictableDelay(dis(gen));
        }
    }
    // cout << "Worker " << id << " done." << endl;
}

/** main function */
int main() {
    for (int run = 0; run < run_times; ++run) {
        counter.store(0, memory_order_relaxed);
        cout << "Run " << run << ": ";
        thread threads[num_threads];

        for (int i = 0; i < num_threads; ++i) {
            threads[i] = thread(worker, i);
        }

        for (int i = 0; i < num_threads; ++i) {
            threads[i].join();
        }

        cout << (max_count == counter.load() ? "Correct" : "Wrong") << endl;
    }

    return 0;
}