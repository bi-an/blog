#include <atomic>

std::atomic<int> a{0};
std::atomic<int> b{0};

void thread1() {
    a.store(1, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_seq_cst);  // 全屏障
    b.store(1, std::memory_order_relaxed);
}

void thread2() {
    while (b.load(std::memory_order_relaxed) == 0);
    std::atomic_thread_fence(std::memory_order_seq_cst);  // 全屏障
    assert(a.load(std::memory_order_relaxed) == 1);
}