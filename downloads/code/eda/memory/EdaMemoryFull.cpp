#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <random>
#include <map>
#include <algorithm>
#include <fstream>

struct Event {
    int time;
    std::function<void()> action;
    bool operator<(const Event& other) const { return time > other.time; }
};

class EdaMemoryFull {
    std::vector<uint8_t> mem;
    std::priority_queue<Event> event_queue;
    int sim_time = 0;
    int read_delay;
    std::mt19937 gen;

    int cache_size;
    struct CacheLine { bool valid=false; int tag=-1; };
    std::vector<CacheLine> cache;

    int max_bus_access_per_cycle = 2;
    int current_cycle_access = 0;

    // 统计信息
    std::map<int,int> read_count, write_count, read_hits, write_hits;
    std::map<int,int> read_delay_total;
    int dynamic_power = 0; // 动态功耗
    int static_power; // 静态功耗
    std::vector<int> dynamic_power_per_cycle; // 每周期动态功耗

public:
    enum WritePriority { WRITE_FIRST, READ_FIRST, ROUND_ROBIN } write_prio;

    EdaMemoryFull(size_t size, int delay, int c_size, WritePriority prio=WRITE_FIRST)
        : mem(size,0), read_delay(delay), gen(std::random_device{}()), 
          cache_size(c_size), cache(c_size), static_power(size/8), write_prio(prio)
    {
        std::uniform_int_distribution<> dis(0,255);
        for(auto &v: mem) v = dis(gen);
    }

    void write(int port,int addr,const std::vector<uint8_t>& data){
        write_count[port]++;
        if(current_cycle_access>=max_bus_access_per_cycle){
            event_queue.push({sim_time+1,[this,port,addr,data](){ write(port,addr,data); }});
            return;
        }
        current_cycle_access++;
        if(write_prio==WRITE_FIRST){
            apply_write(addr,data);
        } else {
            event_queue.push({sim_time+1,[this,addr,data](){ apply_write(addr,data); }});
        }
        update_cache(addr,data,true);
    }

    void read(int port,int addr,size_t length,std::function<void(std::vector<uint8_t>)> callback){
        read_count[port]++;
        if(current_cycle_access>=max_bus_access_per_cycle){
            event_queue.push({sim_time+1,[this,port,addr,length,callback](){ read(port,addr,length,callback); }});
            return;
        }
        current_cycle_access++;
        int trigger_time = sim_time+read_delay;
        bool hit = check_cache(addr,length);
        if(hit) read_hits[port]++;
        event_queue.push({trigger_time,[this,addr,length,callback,port,trigger_time]() {
            std::vector<uint8_t> data;
            for(size_t i=0;i<length;i++){
                uint8_t val = mem[(addr+i)%mem.size()];
                if(random_bit_flip()) val ^= (1<<(gen()%8));
                dynamic_power += count_bit_changes(val, mem[(addr+i)%mem.size()]);
                data.push_back(val);
            }
            read_delay_total[port] += (trigger_time - sim_time);
            callback(data);
        }});
    }

    void tick(){
        sim_time++;
        current_cycle_access = 0;
        int cycle_dyn_power = 0;

        while(!event_queue.empty() && event_queue.top().time <= sim_time){
            auto e = event_queue.top(); event_queue.pop();
            int before = dynamic_power;
            e.action();
            cycle_dyn_power += (dynamic_power - before);
        }

        dynamic_power_per_cycle.push_back(cycle_dyn_power);

        // ASCII 可视化每周期动态功耗
        int scale = 50;
        int bar_len = *std::max_element(dynamic_power_per_cycle.begin(), dynamic_power_per_cycle.end())>0 ?
                      cycle_dyn_power*scale/(*std::max_element(dynamic_power_per_cycle.begin(), dynamic_power_per_cycle.end())) : 0;
        std::cout << "Cycle " << sim_time << " dyn power: " << cycle_dyn_power
                  << " total power: " << dynamic_power + static_power << " ";
        for(int i=0;i<bar_len;i++) std::cout<<"#";
        std::cout << std::endl;
    }

    void print_stats() const {
        std::cout<<"Simulation stats:\n";
        for(auto& [port,cnt]: read_count)
            std::cout<<"Port "<<port<<" read count: "<<cnt
                     <<", hits: "<<read_hits.at(port)
                     <<", avg delay: "<<(cnt?read_delay_total.at(port)/cnt:0)<<"\n";
        for(auto& [port,cnt]: write_count)
            std::cout<<"Port "<<port<<" write count: "<<cnt
                     <<", hits: "<<write_hits.at(port)<<"\n";
        std::cout<<"Dynamic power units: "<<dynamic_power<<"\n";
        std::cout<<"Static power units: "<<static_power<<"\n";
        std::cout<<"Total power units: "<<dynamic_power + static_power<<"\n";
    }

    void export_power_csv(const std::string &filename) const {
        std::ofstream ofs(filename);
        if(!ofs.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }
        ofs << "Cycle,DynamicPower,StaticPower,TotalPower\n";
        for(size_t i=0;i<dynamic_power_per_cycle.size();i++){
            int dyn = dynamic_power_per_cycle[i];
            int total = dyn + static_power;
            ofs << (i+1) << "," << dyn << "," << static_power << "," << total << "\n";
        }
        ofs.close();
        std::cout << "Power data exported to " << filename << std::endl;
    }

private:
    bool random_bit_flip(){
        std::uniform_real_distribution<> dis(0.0,1.0);
        return dis(gen)<0.01;
    }

    int count_bit_changes(uint8_t a,uint8_t b){
        uint8_t diff = a^b;
        int count=0;
        while(diff){ count+=diff&1; diff>>=1; }
        return count;
    }

    bool check_cache(int addr,size_t length){
        int line = addr % cache_size;
        int tag = addr / cache_size;
        return cache[line].valid && cache[line].tag==tag;
    }

    void update_cache(int addr,const std::vector<uint8_t>& data,bool write=false){
        int line = addr % cache_size;
        int tag = addr / cache_size;
        cache[line].valid=true;
        cache[line].tag=tag;
        if(write) write_hits[0]++;
    }

    void apply_write(int addr,const std::vector<uint8_t>& data){
        for(size_t i=0;i<data.size();i++) mem[(addr+i)%mem.size()]=data[i];
    }
};

// 示例主程序
int main(){
    EdaMemoryFull mem(1024,2,16,EdaMemoryFull::WRITE_FIRST);

    mem.write(0,10,{42,43,44});
    mem.write(1,11,{99});
    mem.read(0,10,3,[](std::vector<uint8_t> data){
        std::cout<<"Port0 read burst: ";
        for(auto v:data) std::cout<<(int)v<<" ";
        std::cout<<std::endl;
    });
    mem.read(1,11,1,[](std::vector<uint8_t> data){
        std::cout<<"Port1 read: "<<(int)data[0]<<std::endl;
    });

    for(int i=0;i<10;i++) mem.tick();
    // 导出功耗数据
    mem.export_power_csv("memory_power.csv");

    mem.print_stats();
    return 0;
}
