#include <tbb/tbb.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

std::mutex mtx;

void task_group_function() {
    tbb::task_group tg;
    int max_concurrency = tbb::this_task_arena::max_concurrency();
    {
        std::lock_guard<std::mutex> lock(mtx);
        cout << "Task group max concurrency: " << max_concurrency << endl;
    }
    for (int i = 0; i < 16; ++i) {
        tg.run([i] {
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "Task group thread " << std::this_thread::get_id() << " is running." << std::endl;
            }
            sleep(1);
        });
    }
    tg.wait();
}

void task_arena_function() {
    tbb::task_arena arena(4);
    int max_concurrency = arena.max_concurrency();
    {
        std::lock_guard<std::mutex> lock(mtx);
        cout << "Task arena max concurrency: " << max_concurrency << endl;
    }
    arena.execute([max_concurrency] {
        tbb::parallel_for(0, 16, [](int i) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "Task arena thread " << std::this_thread::get_id() << " is running." << std::endl;
            }
            sleep(2);
        });
    });
}

int main() {
    // 获取默认task_arena的最大并发线程数
    int arena_max_concurrency = tbb::this_task_arena::max_concurrency();
    std::cout << "Default task_arena max concurrency: " << arena_max_concurrency << std::endl;

    // 创建两个线程
    std::thread tg_thread(task_group_function);
    std::thread ta_thread(task_arena_function);

    // 等待两个线程完成
    tg_thread.join();
    ta_thread.join();

    return 0;
}