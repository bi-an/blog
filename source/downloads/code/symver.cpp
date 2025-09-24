/** 指定特定版本的符号 START */

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

__asm__ (".symver _ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE, _ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE@GLIBCXX_3.4.11");

#ifdef __cplusplus
}
#endif /** __cplusplus */

/** 指定特定版本的符号 END */

// 之后就可以使用 std::condition_variable::wait(std::unique_lock<std::mutex>&) 就是 GLIBCXX_3.4.11 的
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
 
std::mutex m;
std::condition_variable cv;
std::string data;
bool ready = false;
bool processed = false;
 
void worker_thread()
{
    // wait until main() sends data
    std::unique_lock lk(m);
    cv.wait(lk, []{ return ready; });
 
    // after the wait, we own the lock
    std::cout << "Worker thread is processing data\n";
    data += " after processing";
 
    // send data back to main()
    processed = true;
    std::cout << "Worker thread signals data processing completed\n";
 
    // manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lk.unlock();
    cv.notify_one();
}
 
int main()
{
    std::thread worker(worker_thread);
 
    data = "Example data";
    // send data to the worker thread
    {
        std::lock_guard lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();
 
    // wait for the worker
    {
        std::unique_lock lk(m);
        cv.wait(lk, []{ return processed; });
    }
    std::cout << "Back in main(), data = " << data << '\n';
 
    worker.join();
}