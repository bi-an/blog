
#ifndef CACHE_SIMULATOR_H
#define CACHE_SIMULATOR_H

#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>

// 缓存行结构体
struct CacheLine {
    bool valid = false;
    bool dirty = false;
    uint32_t tag = 0;
    uint64_t last_used = 0; // 用于LRU替换策略
    std::vector<uint8_t> data;
};

// 端口访问请求结构体
struct PortRequest {
    uint32_t port_id;
    bool is_write;
    uint32_t addr;
    uint8_t* data_ptr;
    size_t data_size;
};

class CacheSimulator {
public:
    CacheSimulator(uint32_t line_size, uint32_t num_lines, uint32_t num_ports);
    
    // 多端口访问接口
    void process_request(const PortRequest& req);
    
    // 缓存配置
    void set_write_policy(bool write_back); 
    void set_replacement_policy(int policy); // 0:LRU, 1:FIFO, 2:Random

private:
    // 内部缓存操作
    bool access_cache(uint32_t port_id, uint32_t addr, bool is_write, uint8_t* data, size_t size);
    void handle_miss(uint32_t port_id, uint32_t addr);
    void evict_line(uint32_t set_idx, uint32_t way_idx);

    // 多端口同步
    std::atomic<uint64_t> global_counter_{0}; // 原子计数器
    std::vector<std::mutex> port_locks_;
    
    // 缓存结构
    uint32_t line_size_;
    uint32_t num_sets_;
    std::vector<std::vector<CacheLine>> cache_;

    // 策略配置
    bool write_back_;
    int replacement_policy_;
};
#endif
