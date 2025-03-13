---
title: TBB入门
date: 2024-02-24 14:59:52
categories: c/cpp
tags: thread
---

## 1. 控制线程数

- Method 1: Use the environment variable `TBB_NUM_THREADS` for the gloabl setting.

```bash
export TBB_NUM_THREADS=4
```

TODO: It doesn't seem to work!

- Method 2: Use `tbb::task_arena` or `tbb::task_scheduler_init` (Deprecated).

TBB will use this setting locally within the scope of the `tbb::task_arena`.

```cpp
#include <tbb/pipeline.h>
// Deprecated:
// #include <tbb/task_scheduler_init.h>
#include <tbb/task_arena.h>

// Define your pipeline body
class MyPipeline {
public:
    void operator() (tbb::flow_control& fc) const {
        // Your pipeline logic here
        // ...
        // Inform the pipeline that there is no more data
        fc.stop();
    }
};

int main() {
    // Deprecated: tbb::task_scheduler_init init(1);
    tbb::task_arena arena(4); // 4 threads
    // Do some tasks:
    tbb::parallel_pipeline(/* max_number_of_live_tokens */ 4, MyPipeline); // FIXME: 似乎需要放入 arena 的 execute 函数中

    return 0;
}
```

## 2. `parallel_for`

用 `my_parallel_for` 模拟 `parallel_for` 的实现：

{% include_code my_parallel_for lang:cpp TBB/my_parallel_for.cpp %}


## 3. TBB线程池

TBB似乎总是使用同一个全局线程池。测试代码如下：

{% include_code lang:cpp TBB/tbb_thread_pool.cpp %}


测试：

```bash
$ mkdir build && cd build && cmake .. && make
$ ./tbb_thread_pool > result.txt
$ cat result.txt | grep running | sort | uniq
Task arena thread 140667163379264 is running.
Task arena thread 140667167639104 is running.
Task arena thread 140667184678464 is running.
Task arena thread 140667201848896 is running.
Task group thread 140667167639104 is running.
Task group thread 140667171898944 is running.
Task group thread 140667176158784 is running.
Task group thread 140667180418624 is running.
Task group thread 140667188938304 is running.
Task group thread 140667210303040 is running.
```

从这两行日志可以看出，arena 和 group 重用了同一个线程 ID ，说明它们同属于同一个全局线程池。

```
Task arena thread 140667167639104 is running.
Task group thread 140667167639104 is running.
```

进一步，我们发现全局线程池中的线程总数是自适应的，比如本例就是 `10` 个，既不是 `task_group` 的 `8` 个，
也不是 `task_arena` 的 `4` 个：

TODO

```bash
$ cat result.txt | grep running | sort | uniq | wc -l
10
```

## 4. 任务调度器（Task Scheduler）

[The Task Scheduler](https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/The_Task_Scheduler.html)

### 基于任务编程（Task-Based Programming）

当追求性能时，推荐以逻辑任务（logical tasks）而不是线程（threads）来编程，有以下原因：

- 将并行性与可用资源匹配
- 更快的任务启动和关闭
- 更有效的评估顺序
- 改进负载均衡
- 更高层的思考


TODO

### 4.1. 任务调度器（Task Scheduler）如何工作

[How Task Scheduler Works](https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/How_Task_Scheduler_Works.html)

#### 4.1.1. 深度优先（depth-first）

每个线程都有自己的双端队列，头部称为 top （也称顶部），尾部称为 bottom （也称底部）。
队列的底部是队列的最深处（最末处），底部任务是最新的，顶部任务是最旧的。

深度优先有以下好处：

- `热点缓存命中`：最新的任务的缓存是最热的，所以优先执行新任务。
- `最小化空间`：广度优先会同时创建指数级数量的共存节点，而深度优先虽然也会创建相同数量的节点，但是只有线性数目的节点会同时共存，因为它创建了其他就绪任务的栈。

生产：当线程产生一个任务时，将其放置到线程自身所有的 deque 的尾部。

消费：当线程执行任务时，根据以下规则顺序选取一个任务：

- 规则1：获取上一个任务返回的任务，如果有；
- 规则2：从线程自己所有的 deque 尾部选取一个任务（即深度优先），如果有；
- 规则3：随机选择一个其他线程的 deque ，从其头部窃取一个任务（即广度优先）。如果被选 deque 为空，则重复本条规则直至成功。


规则1 被称为“任务调度绕行（Task Scheduler Bypass）”。

规则2 是深度优先，这使得当前线程得以不断执行最新的任务直至其完成所有工作。

规则3 是临时的广度优先，它将潜在的并行转化为实际的并行。

#### 4.1.2. 任务调度绕行（Task Scheduler Bypass）技术

一个任务从产生到被执行涉及以下步骤：

> - 将新任务加入线程的 deque 。
> - 执行当前任务直至完成。
> - 从线程 deque 获取一个任务执行，除非该任务被其他线程窃取走了。

其中，步骤1 和 步骤3 会引入不必要的 deque 操作，甚至更糟的是，允许窃取会损害局部性而不会增加显著的并行性。
任务调度器绕行技术可以直接指向下一个要被执行的任务，而不是生产该任务，从而避免了上述问题。
因为根据“规则1”，上一个任务产生的新任务会称为第一个备选任务。
此外，该技术几乎保证了该新任务被当前线程执行，而不是其他线程。

注意：当前唯一能使用该优化技术的是使用 `tbb::task_group` 。


### 指导任务调度器的执行（Guiding Task Scheduler Execution）

[Guiding Task Scheduler Execution](https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/Guiding_Task_Scheduler_Execution.html)

默认情况下，任务计划程序会尝试使用所有可用的计算资源。在某些情况下，您可能希望将任务计划程序配置为仅使用其中的一些资源。

注意：指导任务调度程序的执行可能会导致可组合性问题。

TBB 提供 `task_arena` 接口，通过以下方式指导任务在 arena （竞技场）内被执行:

- 设置首选计算单元；
- 限制部分计算单元。

