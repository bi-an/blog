
#include "cache_simulator.h"
#include <algorithm>
#include <random>
#include <cstring>

CacheSimulator::CacheSimulator(uint32_t line_size, uint32_t num_lines, uint32_t num_ports) 
    : port_locks_(num_ports), line_size_(line_size) {
    num_sets_ = num_lines; // 简单实现，可扩展为组相联
    cache_.resize(num_sets_, std::vector<CacheLine>(1)); // 直接映射
}

void CacheSimulator::process_request(const PortRequest& req) {
    std::lock_guard<std::mutex> lock(port_locks_[req.port_id]);
    access_cache(req.port_id, req.addr, req.is_write, req.data_ptr, req.data_size);
}

bool CacheSimulator::access_cache(uint32_t port_id, uint32_t addr, bool is_write, 
                                uint8_t* data, size_t size) {
    uint32_t tag = addr / line_size_;
    uint32_t set_idx = tag % num_sets_;
    
    // 查找命中
    for (auto& line : cache_[set_idx]) {
        if (line.valid && line.tag == tag) {
            line.last_used = ++global_counter_;
            if (is_write) {
                memcpy(line.data.data(), data, size);
                line.dirty = true;
            } else {
                memcpy(data, line.data.data(), size);
            }
            return true;
        }
    }
    
    // 未命中处理
    handle_miss(port_id, addr);
    return false;
}

void CacheSimulator::handle_miss(uint32_t port_id, uint32_t addr) {
    uint32_t tag = addr / line_size_;
    uint32_t set_idx = tag % num_sets_;
    
    // 查找可替换行
    auto& lines = cache_[set_idx];
    auto victim = std::min_element(lines.begin(), lines.end(),
        [](const CacheLine& a, const CacheLine& b) {
            return a.last_used < b.last_used; // LRU策略
        });
    
    // 写回脏数据
    if (write_back_ && victim->dirty) {
        // 模拟写回主存操作
    }
    
    // 加载新数据
    victim->valid = true;
    victim->tag = tag;
    victim->dirty = false;
    victim->last_used = ++global_counter_;
    // 模拟从主存加载数据
    victim->data.resize(line_size_);
}
