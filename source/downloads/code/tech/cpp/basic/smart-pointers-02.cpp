#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

struct Test {
    Test() { std::cout << "Test constructed\n"; }
    ~Test() { std::cout << "Test destroyed\n"; }
};

void thread_copy(std::shared_ptr<Test>& ptr) {
    // 模拟延迟，试图在主线程销毁后复制
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Thread attempting to copy shared_ptr\n";
    std::shared_ptr<Test> local_copy = ptr;  // 如果 ptr 已经销毁，这里是未定义行为
    if (local_copy) {                        // 可能“成功”复制一个已经销毁的 shared_ptr
        std::cout << "Thread copied shared_ptr\n";
        std::cout << "Use count in thread: "
                  << local_copy.use_count() << "\n";  // 引用计数控制块是一个野指针
    } else {
        std::cout << "Thread failed to copy shared_ptr\n";
    }
}

int main() {
    std::thread t;

    {
        std::shared_ptr<Test> ptr = std::make_shared<Test>();

        t = std::thread(thread_copy, std::ref(ptr));

        // 主线程销毁 shared_ptr
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // 析构，引用计数降为 0，销毁对象
    }

    t.join();
    return 0;
}
