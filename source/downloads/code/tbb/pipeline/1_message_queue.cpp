/**
 * 方案 1：异步队列 + TBB 流水线
 */

#include <tbb/tbb.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "tasks.hpp"

// 全局队列
std::queue<std::vector<char>> readQueue;
std::mutex                    mtx;
std::condition_variable       cv;

// 读线程
void networkReader() {
    while (true) {
        DataChunk chunk;
        read_from_network(chunk);  // 阻塞 I/O
        {
            std::lock_guard<std::mutex> lock(mtx);
            readQueue.push(std::move(chunk.data));
        }
        cv.notify_one();
    }
}

// 压缩任务
void compressor() {
    while (true) {
        std::vector<char> chunk;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return !readQueue.empty(); });
            chunk = std::move(readQueue.front());
            readQueue.pop();
        }

        // CPU 密集计算，使用 TBB 并行
        std::vector<char> compressed(chunk.size());
        tbb::parallel_for(size_t(0), chunk.size(), [&](size_t i) {
            compressed[i] = compress_byte(chunk[i]);  // 假设单字节压缩
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
