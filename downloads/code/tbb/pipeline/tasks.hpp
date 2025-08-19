/**
 * 模拟 IO 和 CPU 任务
 */

#include <unistd.h>

#include <iostream>
#include <vector>

struct DataChunk {
    std::vector<char> data;
    DataChunk() = default;
    explicit DataChunk(size_t size) : data(size) {}
};

struct CompressedChunk {
    std::vector<char> data;
    CompressedChunk() = default;
    explicit CompressedChunk(size_t size) : data(size) {}
};

// 模拟数据读取函数
bool read_from_network(DataChunk& chunk) {
    sleep(3);  // 模拟 IO 延迟

    chunk.data.resize(100);  // 模拟每个数据块有 100 个字节

    static int count = 0;
    if (count++ >= 10) return false;  // 模拟读取 10 个数据块后结束

    std::generate(chunk.data.begin(), chunk.data.end(),
                  []() { return rand() % 256; });
    return true;
}

// 模拟压缩函数
char compress_byte(char byte) {
    for (int i = 0; i < 10'000ll; ++i)
        ;               // 模拟 CPU busy
    return byte % 128;  // 简单压缩算法示例
}

// 模拟写入函数
void write_to_file(const CompressedChunk& chunk) {
    sleep(3);  // 模拟 IO 延迟
    std::cout << "Writing chunk of size " << chunk.data.size() << "\n";
}
