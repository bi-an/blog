
#include "cache_simulator.h"
#include <thread>
#include <iostream>

void port_thread(CacheSimulator& cache, uint32_t port_id) {
    for (int i = 0; i < 1000; ++i) {
        PortRequest req;
        req.port_id = port_id;
        req.is_write = (i % 3 == 0);
        req.addr = rand() % 0xFFFF;
        uint8_t data[64] = {0};
        req.data_ptr = data;
        req.data_size = sizeof(data);
        
        cache.process_request(req);
    }
}

int main() {
    CacheSimulator cache(64, 1024, 4); // 64B行, 1024行, 4端口
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(port_thread, std::ref(cache), i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Cache simulation completed" << std::endl;
    return 0;
}
