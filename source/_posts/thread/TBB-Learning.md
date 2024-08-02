---
title: TBB Learning
date: 2024-02-24 14:59:52
categories: c/cpp
tags: thread
---

# Control the number threads

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
    tbb::parallel_pipeline(/* max_number_of_live_tokens */ 4, MyPipeline);

    return 0;
}
```

