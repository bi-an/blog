class SpinLock {
private:
    std::atomic<bool> flag = {false};
public:
    void lock() {
        bool expected = false;
        while (!flag.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
            expected = false;
        }
    }
    void unlock() {
        flag.store(false, std::memory_order_release);
    }
};