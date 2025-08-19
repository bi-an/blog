/**
 * TBB 流水线
 */

#include <tbb/tbb.h>

#include <iostream>
#include <vector>

#include "tasks.hpp"

int main() {
    tbb::parallel_pipeline(
        /* max_number_of_live_token */ 4,
        // Stage 1: 读网络数据
        tbb::make_filter<void, DataChunk>(
            tbb::filter::serial_in_order,
            [](tbb::flow_control& fc) -> DataChunk {
                DataChunk chunk;
                if (!read_from_network(chunk.data)) {  // 返回 false 时结束
                    fc.stop();
                }
                return chunk;
            }) &
            // Stage 2: CPU 压缩
            tbb::make_filter<DataChunk, CompressedChunk>(
                tbb::filter::parallel,
                [](DataChunk chunk) -> CompressedChunk {
                    CompressedChunk out(chunk.data.size());
                    tbb::parallel_for(size_t(0), chunk.data.size(), [&](size_t i) {
                        out.data[i] = compress_byte(chunk.data[i]);
                    });
                    return out;
                }) &
            // Stage 3: 写文件
            tbb::make_filter<CompressedChunk, void>(
                tbb::filter::serial_in_order,
                [](CompressedChunk out) {
                    write_to_file(out.data);  // 可以异步
                }));

    return 0;
}
