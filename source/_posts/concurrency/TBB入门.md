---
title: TBB 入门
date: 2024-02-24 14:59:52
categories: c/cpp
tags: 线程
---

## 安装

使用脚本一键监查系统是否安装了 TBB

{% include_code lang:bash tbb/check_tbb.sh %}

从包管理器安装：

```bash
sudo apt update
sudo apt install libtbb-dev
```

用源码安装

如果需要最新版本，可以从 GitHub 获取 oneTBB

```bash
git clone https://github.com/oneapi-src/oneTBB.git
cd oneTBB
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

默认会安装到 /usr/local/include/ 和 /usr/local/lib/。

## 文档

官方文档：https://uxlfoundation.github.io/oneTBB/

## 1. TBB 调度原理

###  1.1. TBB 的任务调度模型

#### 1.1.1. 线程池 + 任务队列

- TBB 不让你直接创建线程去执行任务，而是维护一个 固定大小的线程池（数量通常 = CPU 核心数）。
- 所有并行任务都被封装成 task，放入 任务队列。
- 每个工作线程从队列里取任务执行，执行完就取下一个任务。
- 这种模式叫 工作窃取（work-stealing）调度。

#### 1.1.2. 工作窃取机制

- 每个线程有自己的双端队列（deque），优先从自己队列尾部取任务执行。
- 当线程自己的队列空了，会从别的线程队列头部 "窃取" 任务。

优点：

- 动态负载均衡：核心利用率高
- 减少锁竞争：线程多操作自己队列，窃取少量访问其他队列

##### 1.1.2.1. 任务窃取的实现原理

```cpp
tg.run(); // 非阻塞

tg.wait();  // 同步+非阻塞：调用线程会主动参与执行任务，CPU持续运行（非阻塞）
```

**概念澄清**：

- **同步**：必须等所有任务完成。
- **非阻塞**：线程没有闲等，而是参与执行任务。

当 `tg.wait()` 时，调用线程会帮忙执行任务，具体是如何实现的？

**任务队列（Task Queue）**

`tg.run()` 会把所有待执行的任务放入一个共享的队列（通常是无锁队列或工作窃取队列）。

每个工作线程（包括调用线程）都可以从队列里取任务来执行。

**调用线程参与执行**

当调用线程进入 `tg.wait()` 时，它不会只是挂起等待，而是：

- 检查任务队列是否还有未完成的任务。
- 如果有，就直接取出任务并执行。
- 执行完后再继续检查，直到所有任务完成。

**完成条件**

`tg.wait()` 内部会维护一个 **任务计数器**（例如原子变量），记录剩余任务数。

- 每个线程完成一个任务时，计数器减一。
- 当计数器归零时，说明所有任务都完成了，调用线程才真正返回。

**避免死锁**

调用线程必须能执行任务，否则如果它只是等待，而任务又依赖它执行，就会死锁。

所以 `tg.wait()` 的设计就是让调用线程也成为"工作线程"，保证任务能被及时调度。

### 1.2. 与直接使用 std::thread 的区别

| 特性       | std::thread         | TBB              |
| -------- | ------------------- | ---------------- |
| ** 线程数量 ** | 每个任务可能创建新线程         | 线程池固定线程数         |
| ** 调度开销 ** | 每次创建 / 销毁线程成本高        | 任务分配成本低，线程重用     |
| ** 负载均衡 ** | 需要手动管理任务分配          | 自动工作窃取，动态均衡      |
| ** 粒度控制 ** | 线程粒度粗，任务小不能充分利用 CPU | 任务粒度可以小，TBB 自动调度 |
| ** 锁竞争 **  | 共享队列 / 资源容易产生大量锁竞争    | 工作窃取减少锁竞争，性能更高   |


### 1.3. 总结

TBB 高效的核心原因：

- 固定线程池：避免频繁创建销毁线程带来的开销
- 任务化设计：粒度灵活，任务比线程轻量
- 工作窃取：自动负载均衡，提高 CPU 利用率
- 减少锁竞争：每个线程大部分时间只操作自己的队列

所以，用 TBB 写 parallel_for 或 flow::graph 时，即使任务非常小，CPU 核心也能被充分利用，而直接 std::thread 很可能线程创建成本比任务本身还高，性能反而下降。

## 2. task_group

task_group 的本质

- tbb::task_group 是一种 轻量级任务管理器，用来组织一组并行任务。
- 它本身 不创建线程池，也不维护独立线程。
- 当你调用 task_group.run() 或 task_group.wait() 时：
- 任务会被 提交到当前上下文的线程池，通常是 全局线程池
- 由线程池中的工作线程去调度和执行

{% include_code lang:cpp tbb/examples/tbb_thread_pool.cpp %}


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

## 3. task_arena

TBB 是基于任务，不是基于线程。但是如果你想修改 TBB 的线程数，有两种方法：

- 方法一：使用环境变量 `TBB_NUM_THREADS` 进行全局设置：

```bash
export TBB_NUM_THREADS=4
```

TODO: It doesn't seem to work!

- 方法二：使用 `tbb::task_arena` or `tbb::task_scheduler_init` (Deprecated) 进行线程隔离。

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

task_arena 默认 不会去窃取全局线程池的线程

1. task_arena 的线程隔离

tbb::task_arena 是 局部线程池的抽象，可以指定线程数量和优先级。

在 arena.execute(...) 里执行的任务：

优先使用 task_arena 自己的线程（如果 arena 里有空闲线程）

不会去窃取全局线程池的线程

也就是说，task_arena 内的任务和全局线程池是相对隔离的。

2. 嵌套并行和空闲线程利用

如果 task_arena 内的线程空闲不足，默认不会去全局线程池窃取线程。

但是 TBB 内部可能会将一些未使用的线程调度给 arena，但这属于内部优化，不等同于直接窃取整个全局线程池。

总体原则：arena 控制自己的线程数，不影响全局线程池。

3. 全局线程池 vs task_arena 总结

| 特性    | 全局线程池          | task\_arena          |
| ----- | -------------- | -------------------- |
| 线程池数量 | 默认一个           | 每个 arena 独立，可自定义线程数  |
| 窃取行为  | 工作窃取机制（线程之间互窃） | 默认只在 arena 内窃取，不窃取全局 |
| 嵌套并行  | 嵌套任务复用全局线程     | 嵌套任务复用 arena 线程      |
| 适用场景  | 大多数并行调用        | 局部控制线程数、避免与全局任务竞争    |


4. 小结

- 默认情况下，全局线程池是唯一的，TBB 所有普通并行调用都会复用它。
- task_arena 提供局部线程池，在其作用域内执行的任务主要使用 arena 的线程，不去抢全局线程池。
- 用 task_arena 可以做局部限制（比如 GUI 线程或限制 CPU 核心占用），对全局线程池影响很小。

## 4. `parallel_for`

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



## 5. 任务调度器（Task Scheduler）

[The Task Scheduler](https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/The_Task_Scheduler.html)

### 5.1. 基于任务编程（Task-Based Programming）

当追求性能时，推荐以逻辑任务（logical tasks）而不是线程（threads）来编程，有以下原因：

- 将并行性与可用资源匹配
- 更快的任务启动和关闭
- 更有效的评估顺序
- 改进负载均衡
- 更高层的思考


TODO

### 5.2. 任务调度器（Task Scheduler）如何工作

[How Task Scheduler Works](https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/How_Task_Scheduler_Works.html)

#### 5.2.1. 深度优先（depth-first）

每个线程都有自己的双端队列，头部称为 top （也称顶部），尾部称为 bottom （也称底部）。
队列的底部是队列的最深处（最末处），底部任务是最新的，顶部任务是最旧的。

深度优先有以下好处：

- ` 热点缓存命中 `：最新的任务的缓存是最热的，所以优先执行新任务。
- ` 最小化空间 `：广度优先会同时创建指数级数量的共存节点，而深度优先虽然也会创建相同数量的节点，但是只有线性数目的节点会同时共存，因为它创建了其他就绪任务的栈。

生产：当线程产生一个任务时，将其放置到线程自身所有的 deque 的尾部。

消费：当线程执行任务时，根据以下规则顺序选取一个任务：

- 规则 1：获取上一个任务返回的任务，如果有；
- 规则 2：从线程自己所有的 deque 尾部选取一个任务（即深度优先），如果有；
- 规则 3：随机选择一个其他线程的 deque ，从其头部窃取一个任务（即广度优先）。如果被选 deque 为空，则重复本条规则直至成功。


规则 1 被称为 “任务调度绕行（Task Scheduler Bypass）”。

规则 2 是深度优先，这使得当前线程得以不断执行最新的任务直至其完成所有工作。

规则 3 是临时的广度优先，它将潜在的并行转化为实际的并行。

#### 5.2.2. 任务调度绕行（Task Scheduler Bypass）技术

一个任务从产生到被执行涉及以下步骤：

> - 将新任务加入线程的 deque 。
> - 执行当前任务直至完成。
> - 从线程 deque 获取一个任务执行，除非该任务被其他线程窃取走了。

其中，步骤 1 和 步骤 3 会引入不必要的 deque 操作，甚至更糟的是，允许窃取会损害局部性而不会增加显著的并行性。
任务调度器绕行技术可以直接指向下一个要被执行的任务，而不是生产该任务，从而避免了上述问题。
因为根据 “规则 1”，上一个任务产生的新任务会称为第一个备选任务。
此外，该技术几乎保证了该新任务被当前线程执行，而不是其他线程。

注意：当前唯一能使用该优化技术的是使用 `tbb::task_group` 。


### 5.3. 指导任务调度器的执行（Guiding Task Scheduler Execution）

[Guiding Task Scheduler Execution](https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/Guiding_Task_Scheduler_Execution.html)

默认情况下，任务计划程序会尝试使用所有可用的计算资源。在某些情况下，您可能希望将任务计划程序配置为仅使用其中的一些资源。

注意：指导任务调度程序的执行可能会导致可组合性问题。

TBB 提供 `task_arena` 接口，通过以下方式指导任务在 arena （竞技场）内被执行:

- 设置首选计算单元；
- 限制部分计算单元。


### 5.4. 工作隔离（Work Isolation）

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

** 补充：** 让我们通过一个简单的例子来说明隔离区域内其他线程如何生成子任务，并且这些子任务可以由当前线程执行。

假设我们有一个隔离区域，其中有两个线程：线程 A 和线程 B。我们在这个隔离区域内生成了一些任务，并且这些任务可能会生成子任务。

{% include_code TBB/work_isolation_eg5.cpp %}

在这个例子中：

taskA 和 taskB 是在隔离区域内生成的任务。
taskA 生成了两个子任务 Subtask A1 和 Subtask A2。
taskB 生成了两个子任务 Subtask B1 和 Subtask B2。
假设线程 A 执行了 taskA，线程 B 执行了 taskB。在隔离区域内，线程 A 和线程 B 可以执行彼此生成的子任务。例如，线程 A 可以执行 Subtask B1 或 Subtask B2，而线程 B 可以执行 Subtask A1 或 Subtask A2，只要这些子任务属于同一个隔离区域。


## 6. TBB 的无锁设计

严格来说，TBB 并不是完全无锁，但它尽量采用 无锁（lock-free）设计 来提高性能。下面详细说明：

a. 无锁设计的部分

- 工作窃取双端队列（deque）
  - 线程自己的尾部操作（push/pop）通常是 无锁
  - 窃取线程从头部 steal 任务使用 原子 CAS（Compare-And-Swap），也是无锁操作

- 并行算法（parallel_for, parallel_reduce）
  - 内部任务调度和分割通常用无锁队列 + 原子操作

- 轻量级任务对象（task）引用计数
  - 使用原子操作维护任务生命周期，不依赖互斥锁

总结：TBB 在任务调度和工作窃取上，尽量用无锁和原子操作，保证高性能并发执行。

b. 仍然存在锁的场景

- 队列扩容（grow）
  - 当线程队列满，需要扩容底层数组时会使用轻量锁保护

- 某些并行容器
  - concurrent_hash_map, concurrent_vector 等在处理极端并发情况时会使用锁（有时是读写锁）

- 全局管理或 arena 初始化
  - 初始化线程池、任务调度器时可能使用锁，通常只在启动阶段发生

### 6.1. 总结

| 特性                                 | 是否无锁       | 说明            |
| ---------------------------------- | ---------- | ------------- |
| 工作窃取 deque（尾部 push/pop + 头部 steal） | ✅ 无锁（原子操作） | 性能关键部分        |
| 任务引用计数                             | ✅ 原子操作     | 保证任务生命周期安全    |
| 并行算法内部调度                           | ✅ 大部分无锁    | 依赖原子和无锁队列     |
| 队列扩容                               | ⚠️ 使用轻量锁   | 极少发生，性能影响有限   |
| 并行容器                               | ⚠️ 部分锁     | 保障线程安全，复杂容器需要 |


## 7. TBB 的线程生命期

是的，即使你只调用一次 TBB 并行函数，线程池创建后在整个程序运行期间也不会自动销毁或减少线程数。具体说明如下：

1. 全局线程池生命周期

TBB 默认全局线程池是在 第一次使用 TBB 并行接口时创建的。
线程数固定（通常等于 CPU 核心数），整个程序运行期间一直存在。
不会因为你长时间没有提交任务而销毁线程，也不会自动减少线程数。

2. 线程状态

长时间不使用 TBB 时，线程 处于空闲等待状态，不会占用 CPU。
内存占用仍然存在，因为线程堆栈和线程管理结构仍在。
一旦再次调用 TBB 并行函数，线程立即复用，无需重新创建。

3. 可控策略

如果希望线程在长时间不使用时释放资源，可以：
手动控制线程池生命周期（较复杂，需深入 TBB 内部 API，不推荐）
程序设计上在长时间空闲时退出进程，释放所有线程和内存
一般服务器程序会让线程池持续存在，利用 TBB 的工作窃取和线程复用优势，提高后续任务性能。

✅ 总结

TBB 线程池线程数固定，不会因长时间不使用而减少。
线程空闲时不占用 CPU，但占用内存和线程管理资源。
这种设计是为了 提高任务再次执行的响应速度，适合服务器或长期运行的高性能程序。


## 8. TBB 是 CPU 密集型的

如果你的程序有大量 I/O 操作，使用 TBB 的一些特点和注意事项需要特别留意，因为 TBB 是为 CPU 密集型任务 和 任务并行化 设计的。具体分析如下：

1. TBB 对 I/O 的特点

- 线程池固定，线程数通常 = CPU 核心数
- 每个线程会被用来执行任务，如果任务中 发生阻塞 I/O（如文件读写、网络请求）：
- 阻塞线程会占用线程池的一个线程
- 其他任务可能因为线程不足而等待

结论：TBB 默认不适合大量阻塞 I/O 的场景

2. 常见问题

| 问题       | 原因              | 后果                  |
| -------- | --------------- | ------------------- |
| CPU 利用率低 | 阻塞 I/O 占用线程     | 任务等待，性能下降           |
| 线程不足     | 阻塞线程占用全局线程池     | 并行度降低，任务调度受阻        |
| 死锁风险     | TBB 线程池线程被长时间阻塞 | 如果任务依赖其他并行任务，可能互相等待 |

3. 解决方法

a. 分离 I/O 线程

不要在 TBB 线程池中直接做阻塞 I/O
可以开独立线程或线程池专门处理 I/O
TBB 线程只处理 CPU 密集型任务

b. 使用异步 I/O

对网络 / 文件 I/O 使用非阻塞或异步 API（如 asio、io_uring）
任务提交给 TBB 时 立即返回，I/O 完成通过回调或 future 处理

c. task_arena + 限制线程

如果必须在 TBB 线程里做少量阻塞 I/O，可以创建一个 局部 task_arena，限制线程数，避免阻塞全局线程池

d. 混合模型

CPU 密集任务用 TBB
阻塞 I/O 用专门线程池 / 异步框架
最后用 future/promise 或 task_group 协调结果

4. 总结

TBB 是 CPU 密集型并行框架，不适合大量阻塞 I/O
阻塞 I/O 会占用线程池线程，降低并行度

建议：

分离 I/O 线程或线程池
使用异步 I/O
TBB 只处理计算任务

## 9. 推荐阅读

1. Intel Building Blocks 编程指南. James Reinders.
2. Patterns for Parallel Pragramming. Timothy Mattson 等.
3. 设计模式：Design Patterns of Reusable Object-Oriented Software (Addison Wesley). Gamma, Helm, Johnson 和 Vlissides.