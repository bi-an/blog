---
title: 为 lock-free 空闲链表解决 ABA 问题
date: 2025-09-15 20:23:48
categories: 并发编程
tags: ConcurrentQueue
---

# 前言

源码：https://github.com/cameron314/concurrentqueue

原文
：[Solving the ABA Problem for Lock-Free Free Lists](https://moodycamel.com/blog/2014/solving-the-aba-problem-for-lock-free-free-lists)

## 常见的解决方案

### Tagged / versioned 指针

每次修改 head 的同时也变更一个版本号（tag/version），把 (pointer, version) 当作 CAS 的对象。即使指针
回到同一个 A，只要版本号不同，CAS 就会失败，防止 ABA。

缺点包括：在某些机器上需要双字宽度（两个 word）CAS，如果硬件／编译器／平台不支持就只能模拟／锁住；或
者要压缩 pointer 或 tag 大小，可能限制可管理的节点数量／地址空间。

### LL/SC（Load-Linked / Store-Conditional）原语

在支持 LL/SC 的架构上，它自然就可以阻止 ABA：因为 store-conditional 会检测在 load 和 store 之间地址
是否被“写过”，哪怕写了后来改回原来的值也不行。缺点是很多主流架构（尤其 x86）不支持或者支持有限。

## 作者的方法：以引用计数＋“should be on freelist”标志（标志 + 引用计数）

作者提出了一个适用于 free list 的通用方法，结合以下几个机制来避免 ABA，且保持 lock‐free 特性。
