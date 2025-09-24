
// EBR: Epoch-Based Reclamation
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

constexpr int MAX_THREADS = 4;

std::atomic<int> global_epoch{0};
std::atomic<bool> active[MAX_THREADS];
std::atomic<int> local_epoch[MAX_THREADS];

std::mutex retire_mutex;
std::unordered_map<int, std::vector<int*>> retire_list[3];  // 每个 epoch 的垃圾列表

void enter_critical(int tid) {
    active[tid].store(true, std::memory_order_relaxed);
    local_epoch[tid].store(global_epoch.load(std::memory_order_relaxed),
                           std::memory_order_relaxed);
}

void exit_critical(int tid) {
    active[tid].store(false, std::memory_order_relaxed);
}

void retire_node(int tid, int* node) {
    std::lock_guard<std::mutex> lock(retire_mutex);
    int epoch = global_epoch.load(std::memory_order_relaxed);
    retire_list[epoch][tid].push_back(node);
}

void try_advance_epoch() {
    int current = global_epoch.load(std::memory_order_relaxed);
    for (int i = 0; i < MAX_THREADS; ++i) {
        if (active[i].load(std::memory_order_relaxed) &&
            local_epoch[i].load(std::memory_order_relaxed) != current) {
            return;  // 有线程还在旧 epoch，不能推进
        }
    }

    int next_epoch = (current + 1) % 3;
    global_epoch.store(next_epoch, std::memory_order_relaxed);

    // 回收两代前的垃圾
    int reclaim_epoch = (next_epoch + 1) % 3;
    std::lock_guard<std::mutex> lock(retire_mutex);
    for (auto& [tid, nodes] : retire_list[reclaim_epoch]) {
        for (auto node : nodes) {
            delete node;
        }
        nodes.clear();
    }
}
