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

API: [parallel_for](https://oneapi-spec.uxlfoundation.org/specifications/oneapi/latest/elements/onetbb/source/algorithms/functions/parallel_for_func)

1. 用 `my_parallel_for` 模拟 `parallel_for` 的实现：

{% include_code lang:cpp TBB/my_parallel_for.cpp %}

2. 发出任务的线程也会成为工作线程之一，并参与任务的执行，测试代码如下：

{% include_code lang:cpp TBB/test_parallel_for.cpp %}

测试结果：

```bash
$ ./test_parallel_for 
Main thread ID: 140220582070080
Processing data: 2 on thread 140220582070080
Processing data: 6 on thread 140220557755968
Processing data: 4 on thread 140220574795328
Processing data: 8 on thread 140220566275648
Processing data: 10 on thread 140220562015808
```

可见，`data 2` 是由主线程处理的。也就是说，`parallel_for` 虽然被称为 a blocking parallel construt，但线程等待所有任务完成期间是非阻塞的，它还可以充当工作线程执行任务池中的任务。

代码模拟 `parallel_for` 的 `wait` ：

{% include_code TBB/my_task_scheduler.cpp %}

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

### 4.1. 基于任务编程（Task-Based Programming）

当追求性能时，推荐以逻辑任务（logical tasks）而不是线程（threads）来编程，有以下原因：

- 将并行性与可用资源匹配
- 更快的任务启动和关闭
- 更有效的评估顺序
- 改进负载均衡
- 更高层的思考


TODO

### 4.2. 任务调度器（Task Scheduler）如何工作

[How Task Scheduler Works](https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/How_Task_Scheduler_Works.html)

#### 4.2.1. 深度优先（depth-first）

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

#### 4.2.2. 任务调度绕行（Task Scheduler Bypass）技术

一个任务从产生到被执行涉及以下步骤：

> - 将新任务加入线程的 deque 。
> - 执行当前任务直至完成。
> - 从线程 deque 获取一个任务执行，除非该任务被其他线程窃取走了。

其中，步骤1 和 步骤3 会引入不必要的 deque 操作，甚至更糟的是，允许窃取会损害局部性而不会增加显著的并行性。
任务调度器绕行技术可以直接指向下一个要被执行的任务，而不是生产该任务，从而避免了上述问题。
因为根据“规则1”，上一个任务产生的新任务会称为第一个备选任务。
此外，该技术几乎保证了该新任务被当前线程执行，而不是其他线程。

注意：当前唯一能使用该优化技术的是使用 `tbb::task_group` 。


### 4.3. 指导任务调度器的执行（Guiding Task Scheduler Execution）

[Guiding Task Scheduler Execution](https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/Guiding_Task_Scheduler_Execution.html)

默认情况下，任务计划程序会尝试使用所有可用的计算资源。在某些情况下，您可能希望将任务计划程序配置为仅使用其中的一些资源。

注意：指导任务调度程序的执行可能会导致可组合性问题。

TBB 提供 `task_arena` 接口，通过以下方式指导任务在 arena （竞技场）内被执行:

- 设置首选计算单元；
- 限制部分计算单元。


### 4.4. 工作隔离（Work Isolation）

[Work Isolation](https://www.intel.com/content/www/us/en/docs/onetbb/developer-guide-api-reference/2021-12/work-isolation.html)

{% include_code TBB/work_isolation_eg1.cpp %}

如果当前线程被 `parallel_for` “阻塞”（不是真正的阻塞，只能称为 a blocking parallel construct），那么该线程被允许拿取第一个循环的任务来执行。这会导致即使是同一个线程内，也可出现乱序执行的情况。在大多数情况下，这没有什么危害。

但是少数情况可能出现错误，例如一个 thread-local 变量可能会在嵌套并行构造之外意外被更改：

{% include_code TBB/work_isolation_eg2.cpp %}

在其它场景下，这种行为可能会导致死锁或其他问题。在这些情况下，需要更有力地保证线程内的执行次序。为此，TBB 提供了一些隔离并行构造的执行的方法，以使其任务不会干扰其他同时运行的任务。

其中一种方法是在单独的 `task_arena` 中执行内层循环：

{% include_code TBB/work_isolation_eg3.cpp %}

然而，使用单独的 arena 进行工作隔离并不总是方便的，并且可能会产生明显的开销。为了解决这些缺点，TBB 提供 `this_task_arena::isolate` 函数，通过限制调用线程仅处理在函数对象范围内（也称为隔离区域）安排的任务，来隔离地运行一个用户提供的函数对象。

当一个线程进入一个任务等待调用或（等待）在一个隔离区域内的阻塞并行结构时，该线程只能执行在该隔离区域内生成的任务及其由其他线程生成的子任务（换句话说，即使子任务是由其他线程生成的，只要属于当前隔离区域，当前线程也可以执行这些任务）。线程被禁止执行任何外层任务或属于其他隔离区域的任务。

下面的示例展示了 `this_task_arena::isolate` 的使用，以保证在嵌套的并行结构调用时， thread-local 变量不会被意外修改：

{% include_code TBB/work_isolation_eg4.cpp %}

**补充：**让我们通过一个简单的例子来说明隔离区域内其他线程如何生成子任务，并且这些子任务可以由当前线程执行。

假设我们有一个隔离区域，其中有两个线程：线程A和线程B。我们在这个隔离区域内生成了一些任务，并且这些任务可能会生成子任务。

{% include_code TBB/work_isolation_eg5.cpp %}

在这个例子中：

taskA 和 taskB 是在隔离区域内生成的任务。
taskA 生成了两个子任务 Subtask A1 和 Subtask A2。
taskB 生成了两个子任务 Subtask B1 和 Subtask B2。
假设线程A执行了 taskA，线程B执行了 taskB。在隔离区域内，线程A和线程B可以执行彼此生成的子任务。例如，线程A可以执行 Subtask B1 或 Subtask B2，而线程B可以执行 Subtask A1 或 Subtask A2，只要这些子任务属于同一个隔离区域。


## 5. 推荐阅读

### 5.1. 书籍

1. Intel Building Blocks 编程指南. James Reinders.
2. Patterns for Parallel Pragramming. Timothy Mattson 等.
3. 设计模式：Design Patterns of Reusable Object-Oriented Software (Addison Wesley). Gamma, Helm, Johnson 和 Vlissides.