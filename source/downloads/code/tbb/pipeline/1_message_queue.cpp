/**
 * 方案 1：异步队列 + TBB 流水线
 */

#include <tbb/tbb.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <atomic>

#include "tasks.hpp"

// 全局队列
std::queue<std::vector<char>> readQueue;
std::mutex mtx;
std::condition_variable cv;
std::atomic_bool stop = false;

// 读线程
void networkReader() {
    while (!stop) {
        DataChunk chunk;

        if (!read_from_network(chunk))  // 阻塞 I/O
        {
            stop = true;
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            readQueue.push(std::move(chunk.data));
        }
        cv.notify_one();
    }
}

// 压缩任务
void compressor() {
    while (!stop) {
        DataChunk chunk;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return !readQueue.empty(); });
            chunk.data = std::move(readQueue.front());
            readQueue.pop();
        }

        // CPU 密集计算，使用 TBB 并行
        CompressedChunk compressed(chunk.data.size());
        tbb::parallel_for(size_t(0), chunk.data.size(), [&](size_t i) {
            compressed.data[i] = compress_byte(chunk.data[i]);  // 假设单字节压缩
        });

        write_to_file(compressed);  // 可以异步
    }
}

int main() {
    std::thread reader(networkReader);
    std::thread worker(compressor);

    reader.join();
    worker.join();
}
