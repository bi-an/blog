/**
 * 模拟 IO 和 CPU 任务
 */

#include <unistd.h>

#include <vector>

struct DataChunk {
    std::vector<char> data;
};
struct CompressedChunk {
    std::vector<char> data;
};

// 阻塞网络 I/O
bool read_from_network(DataChunk& chunk) {
    std::vector<char> buffer(1024, '8');
    chunk.data = buffer;
    sleep(3);  // 模拟延迟
    return true;
}

// CPU 任务： 假设单字节压缩
char compress_byte(char c) {
    for (int i = 0; i < 1'000'0ll; ++i)
        ;  // 模拟 CPU 运行
    return c;
}

// 文件 I/O
void write_to_file(const std::vector<char>& compressed) {
    sleep(3);  // 模拟延迟
}
