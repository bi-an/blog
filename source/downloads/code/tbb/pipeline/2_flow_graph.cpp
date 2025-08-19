
#include <tbb/flow_graph.h>
#include <tbb/tbb.h>

#include <iostream>
#include <vector>

#include "tasks.hpp"

using namespace tbb;
using namespace tbb::flow;

int main() {
    graph g;

    // 1. 读取节点（串行）
    input_node<DataChunk> reader(
        g,
        [](flow_control& fc) -> DataChunk {
            DataChunk chunk(1024);  // 1KB数据块
            if (!read_from_network(chunk)) {
                fc.stop();
                return DataChunk();
            }
            return chunk;
        });

    // 2. 并行处理节点（无限制并发）
    function_node<DataChunk, CompressedChunk> processor(
        g, unlimited,
        [](const DataChunk& input) -> CompressedChunk {
            CompressedChunk output(input.data.size());

            tbb::parallel_for(
                tbb::blocked_range<size_t>(0, input.data.size()),
                [&](const tbb::blocked_range<size_t>& r) {
                    for (size_t i = r.begin(); i != r.end(); ++i) {
                        output.data[i] = compress_byte(input.data[i]);
                    }
                });

            return output;
        });

    // 3. 写入节点（串行保证写入顺序）
    function_node<CompressedChunk> writer(
        g, serial,
        [](const CompressedChunk& output) {
            write_to_file(output);
        });

    // 构建数据流管道
    make_edge(reader, processor);
    make_edge(processor, writer);

    // 启动管道
    reader.activate();
    g.wait_for_all();

    return 0;
}
