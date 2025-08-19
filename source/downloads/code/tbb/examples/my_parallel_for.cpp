#include <tbb/tbb.h>
#include <vector>
#include <iostream>

// 模拟 parallel_for 的内部实现
void my_parallel_for(const tbb::blocked_range<size_t>& range, const std::function<void(const tbb::blocked_range<size_t>&)>& body) {
    if (range.is_divisible()) {
        // 分割范围
        tbb::blocked_range<size_t> left(range.begin(), range.begin() + (range.end() - range.begin()) / 2);
        tbb::blocked_range<size_t> right(left.end(), range.end());

        // 递归调用
        tbb::parallel_invoke(
            [&] { my_parallel_for(left, body); },
            [&] { my_parallel_for(right, body); }
        );
    } else {
        // 处理当前范围
        body(range);
    }
}

int main() {
    std::vector<int> data(100);
    for (int i = 0; i < 100; ++i) {
        data[i] = i;
    }

    // 使用自定义的 parallel_for 进行并行处理
    my_parallel_for(tbb::blocked_range<size_t>(0, data.size()), [&](const tbb::blocked_range<size_t>& r) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            data[i] *= 2; // 示例操作：将每个元素乘以2
        }
    });

    // 输出结果
    for (const auto& val : data) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return 0;
}