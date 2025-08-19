/**
 * 方案 2：TBB Flow Graph
 */

#include <tbb/tbb.h>

#include "tasks.hpp"

int main() {
    tbb::flow::graph g;
    tbb::flow::source_node<DataChunk> reader(
        g, [](DataChunk& chunk) {
            read_from_network(chunk);  // 阻塞 I/O
            return true;
        },
        false);

    tbb::flow::function_node<DataChunk, CompressedChunk> compressor_node(
        g, tbb::flow::unlimited,
        [](DataChunk chunk) {
            CompressedChunk out(chunk.size());
            tbb::parallel_for(size_t(0), chunk.size(), [&](size_t i) {
                out[i] = compress_byte(chunk[i]);
            });
            return out;
        });

    tbb::flow::function_node<CompressedChunk> writer_node(
        g, tbb::flow::serial,
        [](CompressedChunk out) {
            write_to_file(out);  // 可以异步
        });

    tbb::flow::make_edge(reader, compressor_node);
    tbb::flow::make_edge(compressor_node, writer_node);
    reader.activate();
    g.wait_for_all();
}