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
    std::shared_ptr<Test> local_copy = ptr;  // 资源可能已经释放
    if (local_copy) {      // shared_ptr 还存在，所以可以用来检查资源是否有效
        std::cout << "Thread copied shared_ptr\n";
    } else {
        std::cout << "Thread failed to copy shared_ptr\n";
    }
}

int main() {
    std::shared_ptr<Test> ptr = std::make_shared<Test>();

    std::thread t(thread_copy, std::ref(ptr));

    // 主线程销毁 shared_ptr
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ptr.reset();  // 引用计数降为 0，销毁对象

    t.join();
    return 0;
}
