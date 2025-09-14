---
title: 单生产者 - 单消费者 无锁队列
date: 2025-09-13 17:09:37
categories: concurrency
tags:
---

# 前言

这是阅读 Cameron Desrochers 的 [A Fast Lock-Free Queue for C++](https://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++) 源码的笔记。

仓库地址：https://github.com/cameron314/readerwriterqueue

其他参考文献：

[An Introduction to Lock-Free Programming](https://preshing.com/20120612/an-introduction-to-lock-free-programming/)
[C++ and Beyond 2012: Herb Sutter - atomic Weapons 1 of 2](https://www.youtube.com/watch?v=A8eCGOqgvH4&list=PLjwoip0ltHwHCe0Nw7PDPtzlJP7rUouRL)
[C++ and Beyond 2012: Herb Sutter - atomic Weapons 2 of 2](https://www.youtube.com/watch?v=KeLBd2EJLOU)


## 内存屏障

约束 memory loads/stores 的顺序。

- releaase 内存屏障：告诉 CPU，如果屏障之后的任何写入变得可见，那么屏障之前的任何写入都应该在其他核心中可见，前提是其他核心在读取 写屏障之后写入的数据 后执行读屏障。
  换句话说，如果线程 B 可以看到在另一个线程 A 上的写屏障之后写入的新值，那么在执行读屏障（在线程 B 上）之后，可以保证在线程 A 上的写屏障之前发生的所有写入在线程 B 上可见。


## 实现细节

1. block: 一个连续的环形缓冲区，用来存储元素。这样可以预分配内存。
2. 块过小（这不利于无锁）时，无需将所有现有元素复制到新的块中；多个块（大小独立）以循环链表的形式链接在一起。
3. 当前插入的块称为 “尾块”，当前消费的块称为 “头块”。
4. 头索引指向下一个要读取的满槽；尾索引指向下一个要插入的空槽。如果两个索引相等，则块为空（确切地说，当队列已满时，恰好有一个插槽为空，以避免在具有相同头和尾索引的满块和空块之间产生歧义）。
5. 为了允许队列对任意线程创建 / 析构（独立于生产 / 消费线程），全内存屏障（memory_order_acq_cst）被用在析构函数地最后、析构函数的开头（这会强制所有的 CPU cores 同步 outstanding changes）。显然，在析构函数可以被安全地调用之前，生产者和消费者必须已经停止使用该队列。

## Give me the codes

1. 用户不需要管理内存。
2. 预分配内存，在连续的块中。
3. `try_enqueue`: 保证不会分配内存（队列有初始容量）；
4. `enqueue`: 会根据需要动态扩容。
5. 没有使用 CAS loop；这意味者 enqueue 和 dequeue 是 O(1) 的（没有计入内存分配的时间）。
6. 因为在 x86 平台，内存屏障是空操作，所以 enqueue 和 dequeue 是一系列简单的 loads 和 stores (and branches) 。

此代码仅仅适用于以原子方式处理 自然对齐的整型（aligned integer） 和 原生指针大小（native-pointer-size） 的 loads/stores 的 CPU 上；
幸运的是，这包括了所有的现代处理器（包括 ARM, x86/x86_64 和 PowerPC）。
它不是为在 DEC Alpha 上运行而设计的（DEC Alpha 似乎具有有史以来最弱的内存排序保证）。

注：在 x86 上，memory_order_acquire/release 通常不需要额外指令就能实现语义，但仍然能限制编译器的重排。
fetch_add 不是一个原子操作，而是三个：load, add, store. 所以不适用上述说的 “自然对齐的整型” 或“原生指针大小”的 load/store.

## 性能优化点

1. 平凡析构：跳过析构，直接释放内存。
2. [MCRingBuffer paper](http://www.cse.cuhk.edu.hk/~pclee/www/pubs/ancs09poster.pdf)
   1. cache line padding
   2. local control variables
      1. 减少对全局 read/write 指针的读取
   3. local block

