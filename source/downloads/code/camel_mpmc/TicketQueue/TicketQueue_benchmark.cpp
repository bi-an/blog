#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

constexpr size_t BLOCK_SIZE = 64;
constexpr int N = 1'000'000;
constexpr int NUM_PRODUCERS = 8;
constexpr int NUM_CONSUMERS = 8;
constexpr int ITEMS_PER_PRODUCER = N / NUM_PRODUCERS;

struct Slot {
    std::atomic<bool> ready;
    int data;
};

struct Block {
    Slot slots[BLOCK_SIZE];
};

// BlockQueue: 分块但无 ticket
class BlockQueue {
public:
    BlockQueue() : head(0), tail(0) {
    }

    void enqueue(int value) {
        size_t index = head.fetch_add(1) % BLOCK_SIZE;
        block.slots[index].data = value;
        block.slots[index].ready.store(true, std::memory_order_release);
    }

    bool try_dequeue(int& value) {
        size_t index = tail.fetch_add(1) % BLOCK_SIZE;
        if (!block.slots[index].ready.load(std::memory_order_acquire))
            return false;
        value = block.slots[index].data;
        return true;
    }

private:
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
    Block block;
};

// TicketQueue: 分块 + ticket
class TicketQueue {
public:
    TicketQueue() : head(0), tail(0) {
    }

    void enqueue(int value) {
        size_t ticket = head.fetch_add(1);
        size_t index = ticket % BLOCK_SIZE;
        block.slots[index].data = value;
        block.slots[index].ready.store(true, std::memory_order_release);
    }

    bool try_dequeue(int& value) {
        size_t ticket = tail.fetch_add(1);
        size_t index = ticket % BLOCK_SIZE;
        while (!block.slots[index].ready.load(std::memory_order_acquire)) {
            // 自旋等待
        }
        value = block.slots[index].data;
        return true;
    }

private:
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
    Block block;
};

// 基准测试函数
template <typename QueueType>
double benchmark(const std::string& name, double& opsPerSec) {
    QueueType queue;
    std::atomic<int> totalConsumed{0};
    std::map<int, double> threadWaitTimes;

    auto start = std::chrono::high_resolution_clock::now();

    // 启动生产者线程
    std::vector<std::thread> producers;
    for (int p = 0; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&queue, p]() {
            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                queue.enqueue(i + p * ITEMS_PER_PRODUCER);
            }
        });
    }

    // 启动消费者线程
    std::vector<std::thread> consumers;
    for (int c = 0; c < NUM_CONSUMERS; ++c) {
        consumers.emplace_back([&queue, &totalConsumed, c, &threadWaitTimes]() {
            int item;
            auto localStart = std::chrono::high_resolution_clock::now();
            while (true) {
                auto t0 = std::chrono::high_resolution_clock::now();
                while (!queue.try_dequeue(item)) {
                    // busy wait
                }
                auto t1 = std::chrono::high_resolution_clock::now();
                threadWaitTimes[c] +=
                    std::chrono::duration<double>(t1 - t0).count();

                if (++totalConsumed >= N)
                    break;
            }
            auto localEnd = std::chrono::high_resolution_clock::now();
            double threadTime =
                std::chrono::duration<double>(localEnd - localStart).count();
            std::cout << "Consumer " << c << " finished in " << threadTime
                      << "s, wait time: " << threadWaitTimes[c] << "s\n";
        });
    }

    for (auto& t : producers)
        t.join();
    for (auto& t : consumers)
        t.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    opsPerSec = N / elapsed.count();

    std::cout << "\n"
              << name << " completed in " << elapsed.count()
              << "s, throughput: " << opsPerSec << " ops/sec\n";

    return elapsed.count();
}

int main() {
    double opsBlock = 0.0, opsTicket = 0.0;
    double timeBlock = benchmark<BlockQueue>("BlockQueue", opsBlock);
    double timeTicket = benchmark<TicketQueue>("TicketQueue", opsTicket);
    double speedup = opsTicket / opsBlock;

    std::cout << "\n📊 Performance Comparison\n";
    std::cout << std::left << "| " << std::setw(14) << "Queue Type"
              << "| " << std::setw(12) << "Time (s)"
              << "| " << std::setw(20) << "Throughput (ops/s)"
              << "| " << std::setw(10) << "Speedup"
              << "|\n";

    std::cout << std::string(70, '-') << "\n";

    // BlockQueue row
    std::cout << std::left << "| " << std::setw(14) << "BlockQueue"
              << "| " << std::setw(12) << std::fixed << std::setprecision(6)
              << timeBlock << "| " << std::setw(20) << std::fixed
              << std::setprecision(0) << opsBlock << "| " << std::setw(10)
              << "1.00×"
              << "|\n";

    // TicketQueue row
    std::ostringstream speedupStream;
    speedupStream << std::fixed << std::setprecision(2) << speedup << "×";

    std::cout << std::left << "| " << std::setw(14) << "TicketQueue"
              << "| " << std::setw(12) << std::fixed << std::setprecision(6)
              << timeTicket << "| " << std::setw(20) << std::fixed
              << std::setprecision(0) << opsTicket << "| " << std::setw(10)
              << speedupStream.str() << "|\n";

    return 0;
}
