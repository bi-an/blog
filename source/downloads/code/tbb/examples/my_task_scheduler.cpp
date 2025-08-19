#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>
#include <queue>

class TaskScheduler {
public:
    TaskScheduler(size_t numThreads);
    ~TaskScheduler();
    void enqueue(std::function<void()> task);
    void wait();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::condition_variable finished;
    bool stop;
    size_t activeTasks;

    void workerThread();
    void executeTask();
};

TaskScheduler::TaskScheduler(size_t numThreads) : stop(false), activeTasks(0) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&TaskScheduler::workerThread, this);
    }
}

TaskScheduler::~TaskScheduler() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void TaskScheduler::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void TaskScheduler::wait() {
    std::unique_lock<std::mutex> lock(queueMutex);
    while (!tasks.empty() || activeTasks > 0) {
        // 如果还有任务，执行一个任务，避免当前线程被阻塞
        if (!tasks.empty()) {
            executeTask();
        } else {
            finished.wait(lock);
        }
    }
}

void TaskScheduler::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
            ++activeTasks;
        }
        task();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            --activeTasks;
            if (tasks.empty() && activeTasks == 0) {
                finished.notify_all();
            }
        }
    }
}

void TaskScheduler::executeTask() {
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (tasks.empty()) return;
        task = std::move(tasks.front());
        tasks.pop();
        ++activeTasks;
    }
    task();
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        --activeTasks;
        if (tasks.empty() && activeTasks == 0) {
            finished.notify_all();
        }
    }
}

void parallel_for(int start, int end, std::function<void(int)> func) {
    static TaskScheduler scheduler(std::thread::hardware_concurrency());
    for (int i = start; i < end; ++i) {
        scheduler.enqueue([i, &func] { func(i); });
    }
    scheduler.wait();
}

int main() {
    const int N1 = 100;
    const int N2 = 100;

    // The first parallel loop.
    parallel_for(0, N1, [&](int i) {
        // The second parallel loop.
        parallel_for(0, N2, [&](int j) {
            // Some work
        });
        // 线程发出 parallel_for 之后，需要等待内部所有 parallel loop 的任务完成
        // 在此期间允许继续拿取外部的 parallel loop 的任务执行
    });

    return 0;
}