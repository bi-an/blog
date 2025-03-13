---
title: TBB Learning
date: 2024-02-24 14:59:52
categories: c/cpp
tags: thread
---

## Control the number threads

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

## `parallel_for`

用 `my_parallel_for` 模拟 `parallel_for` 的实现：

{% include_code my_parallel_for lang:cpp TBB/my_parallel_for.cpp %}


## TBB线程数

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
