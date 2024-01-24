---
title: Reentrant Function
date: 2024-01-14 17:44:16
categories: c/cpp
tags: concepts
---

## Concept

**可重入函数**：如果一个函数在执行过程中被中断服务程序打断，执行中断服务程序之后恢复执行，还能不妨碍之前的执行，就称该函数是**可重入的**。

可重入函数一般用于**硬件中断处理**或**递归**等应用程序中。

**可重入程序/可重入子例程**：在多个处理器上能被安全地多次并发调用。

**与线程安全的区别**：可重入函数的概念在多任务操作系统出现之前就存在了，所以该概念仅仅针对的是单线程执行过程。
一个函数可以是线程安全但非可重入的，例如，该函数每次都使用互斥量来包裹。但是，如果该函数用于中断服务程序，那么，它可能在等待第一次执行过程释放互斥量时陷入饥饿。**TODO：陷入饥饿为什么就不是可重入了？**

## Reference

- [Reentrant Function](https://www.geeksforgeeks.org/reentrant-function/)
- [Reentrancy](https://en.wikipedia.org/wiki/Reentrancy_(computing))
