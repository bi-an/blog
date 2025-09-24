#include <tbb/tbb.h>

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;
using namespace tbb;

static std::atomic<int> total_blocks(0);  // Atomic counter to track the number of tasks processed
static std::atomic<int> total_blocks2(0);  // Atomic counter to track the number of tasks processed

// Function to process each data element
void process_data(int i, std::mutex& mtx) {
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Simulate some processing time
    for (int i = 0; i < 100; ++i)
        ;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <number_of_elements>"
                  << "<grain_size>" << std::endl;
        return 1;
    }

    int num_elements = std::stoi(argv[1]);
    int grain_size = std::stoi(argv[2]);

    std::mutex mtx;
    std::vector<int> data(num_elements);
    for (int i = 0; i < num_elements; ++i) {
        data[i] = i;
    }

    // {
    //     std::lock_guard<std::mutex> lock(mtx);
    //     std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;
    // }

    tbb::concurrent_unordered_map<std::thread::id, tbb::concurrent_vector<int>> thread_task_counts;  // To store task counts for each thread
    tbb::concurrent_unordered_map<std::thread::id, tbb::concurrent_vector<int>> thread_task_counts2;  // To store task counts for each thread

    tbb::parallel_for(0, static_cast<int>((data.size() + grain_size - 1) / grain_size), [&](int i) {
        total_blocks++;
        int cnt = 0;  // Thread-local variable to avoid data
        for (int j = i * grain_size; j < std::min(static_cast<int>(data.size()), (i + 1) * grain_size); ++j) {
            process_data(i, mtx);
            ++cnt;
        }
        thread_task_counts[std::this_thread::get_id()].push_back(cnt);
    });

    tbb::parallel_for(blocked_range<int>(0u, data.size(), grain_size), [&](const blocked_range<int>& r) {
        total_blocks2++;
        int cnt = 0;  // Thread-local variable to avoid data
        for (int i = r.begin(); i < r.end(); ++i) {
            process_data(i, mtx);
            ++cnt;
        }
        thread_task_counts2[std::this_thread::get_id()].push_back(cnt);
    });

    std::cout << "Total blocks processed: " << total_blocks.load() << std::endl;
    for (const auto& pair : thread_task_counts) {
        std::cout << "Thread " << pair.first << " processed: ";
        for (const auto& task_count : pair.second)
            std::cout << task_count << ", ";
        std::cout << std::endl;
    }

    std::cout << "Total blocks2 processed: " << total_blocks2.load() << std::endl;
    for (const auto& pair : thread_task_counts2) {
        std::cout << "Thread " << pair.first << " processed: ";
        for (const auto& task_count : pair.second)
            std::cout << task_count << ", ";
        std::cout << std::endl;
    }

    return 0;
}