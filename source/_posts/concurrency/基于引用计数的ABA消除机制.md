---
title: 基于引用计数的 ABA 消除机制
date: 2025-09-15 20:23:48
categories: 并发编程
tags: ConcurrentQueue
---

# 前言

源码：https://github.com/cameron314/concurrentqueue

原文
：[Solving the ABA Problem for Lock-Free Free Lists](https://moodycamel.com/blog/2014/solving-the-aba-problem-for-lock-free-free-lists)

## 共识

- CAS 足以保证线程安全；
- 我们要做的是消除 CAS 的 ABA 问题。

## 常见的解决方案

### Tagged / versioned 指针

每次修改 head 的同时也变更一个版本号（tag/version），把 (pointer, version) 当作 CAS 的对象。即使指针
回到同一个 A，只要版本号不同，CAS 就会失败，防止 ABA。

缺点包括：在某些机器上需要双字宽度（两个 word）CAS，如果 硬件 / 编译器 / 平台 不支持就只能 模拟 / 锁
住；或者要压缩 pointer 或 tag 大小，可能限制可管理的节点数量 / 地址空间。

### LL/SC（Load-Linked / Store-Conditional）原语

在支持 LL/SC 的架构上，它自然就可以阻止 ABA：因为 store-conditional 会检测在 load 和 store 之间地址
是否被“写过”，哪怕写了后来改回原来的值也不行。缺点是很多主流架构（尤其 x86）不支持或者支持有限。

## 作者的方法：以引用计数＋“should be on freelist”标志（标志 + 引用计数）

作者提出了一个适用于 free list 的通用方法来避免 ABA，且保持 lock‐free 特性。

提示：该代码应该从 `try_get()` 开始阅读，然后回到 `add()` ，这样才能理解设计引用计数的意图，否则很容
易迷惑。

- 引用计数是状态标志：
  - 表示当前有多少线程正在操作节点；
  - **空闲链表本身也持有一个对节点的引用计数**。
- 任何线程尝试摘取空闲链表的节点（总是从头部开始）时（`try_get()`）：
  - 必须先增加 head 的引用计数（不同线程可能并发地将节点的引用计数增加到某个值）；→ `(3)`
  - next = head->next；
  - 然后 CAS 竞争，尝试将 freeListHead 调整为 next；
    - 如果 CAS 竞争成功，则成功将 freeListHead 调整为 next，该节点被摘下；引用计数减 2（自己增加 的
      引用计数 + 链表本身持有的引用计数），我们称该线程为`线程 1`。→ `(1)`
    - 如果 CAS 竞争失败，则回退：将自己所增加的引用计数减去；用 CAS 返回的新的 head 重试（下一次获取
      新的 head 节点），这是`线程 2`；→ `(2)`
    - 注意：这两步之间以及每一步自身内，对引用计数的操作都有中间状态，但是此时引用计数必然大于 0 （
      **重点！** ）。
- **ABA 问题**：如果成功摘取节点的`线程 1`完成节点的使用，将节点又添加回空闲链表（`add()`）：
  - 它会发现某个线程正在第 `(2)` 步的回退过程中（还没有将自身增加的引用计数减回去），所以此时引用计
    数还是大于 0（也就是说`线程 1`使用该节点的整个过程，该节点的引用计数都是大于 0 的，但是不影响）
    。
  - 此时它可以选择自旋地等待引用计数降为 0 （即所有之前在 `try_get()` 中 CAS 竞争失败的线程（
    如`线程 2`）回退成功）。因为只有引用计数降为 0 了，才能说明自己是唯一一个尝试在链表上操作该节点
    的线程。但是这从技术上说，就是一个 lock。
  - 作者设计了一个更聪明的办法：`add()` 设置一个 `SHOULD_BE_ON_FREELIST` 标志，然后直接放弃
    （`add()`的任务已经完成了）。
  - 让处于回退中的 `线程 2`，在回退结束的时候，检查 `SHOULD_BE_ON_FREELIST` 标志，如果有该标志，并且
    自己就是最后一个操作该节点的线程的话（即此时的引用计数已经降为 0 了），就帮助 `add()` 把该节点添
    加到 free list。

### 引用计数

现在我们回过头来看引用计数的作用：

- 如果 `refs > 0`：

  - (1) 要么该节点在链表上；
  - (2) 要么有线程正在操作该节点（准确来讲，是在 `try_get()`）；
  - (3) 要么以上两种情况同时存在。
  - 注：(1)(2) 可以只存在一种，见上面的 ABA 问题的描述。

- 如果 `refs == 0`：
  - 节点已经被从空闲链表上取下；
  - 并且其他线程都已经从 `try_get()` 成功回退了。

如果第三个线程，与`线程 1`、`线程 2`一起进入上面的 `try_get()` 竞争，但是当`线程 3`即将执行步
骤`(3)`时，发现 `refs == 0`，它就不能够再增加 head 的引用计数了，因为节点已经被成功取下。否则当它成
功增加引用计数，再去拿取 head->next 的时候，是未定义行为。

正是基于此，在步骤`(3)`增加引用计数的时候，我们需要判断 `refs == 0 ?` 和使用 CAS 增加引用计数，如果
不符合预期，则放弃增加，进入下一轮重试。

```cpp
if ((refs & REFS_MASK) == 0 || !head->freeListRefs.compare_exchange_strong(refs, refs + 1, std::memory_order_acquire)) {
	head = freeListHead.load(std::memory_order_acquire);
	continue;
}
```
