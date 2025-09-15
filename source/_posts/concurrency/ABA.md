---
title: CAS 的 ABA问题
date: 2025-09-15 19:48:58
categories: 并发编程
tags:
---

## 解决方案

🔹 Hazard Pointer（危险指针）

思路：
每个线程在访问共享指针之前，把自己正在访问的指针写到一个全局可见的“hazard pointer”里。

作用：
其他线程在想要回收这个节点内存时，必须检查所有线程的 hazard pointers，如果发现有人还在用这个节点，就不能释放。

优点：简单直接，内存可以安全回收。

缺点：维护 hazard pointers 有一定开销，每次回收都要检查所有线程。

🔹 Epoch-Based Reclamation（基于世代的回收，简称 Epoch GC）

思路：
把时间切分成 epoch（世代）。线程进入临界区时声明自己在某个 epoch。
当一个节点被删除后，先放到一个“延迟回收队列”，等到所有线程都离开这个 epoch 之后，才能真正释放这些节点。

作用：
保证没有线程会在旧 epoch 中访问到已经释放的节点。

优点：比 hazard pointer 更高效（不用逐个检查指针）。

缺点：需要所有线程都周期性地报告自己活跃的 epoch，否则内存可能迟迟回收不了。

🚩 为什么会和 ABA 有关？

像 Michael-Scott 队列这种链表结构，节点被 pop 出队后地址可能被重用。
如果没有安全的内存回收，另一个线程可能 CAS 成功指向了一个“已经被释放并重用的地址”，这就是 ABA 的根源。
所以 hazard pointer 或 epoch GC 是在链表队列里用来避免这种 悬空引用 + ABA 的。

而 moodycamel::ConcurrentQueue 因为用的是 环形 buffer + sequence number，节点不会反复 malloc/free，所以根本就不需要 hazard pointer 或 epoch GC。