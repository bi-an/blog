---
title: 多生产者 - 多消费者 无锁队列
date: 2025-09-14 15:05:08
categories: concurrency
tags:
---

# 前言

这是阅读 Cameron Desrochers 的 [A Fast General Purpose Lock-Free Queue for C++](https://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++) 源码的笔记。

[Detailed Design of a Lock-Free Queue](https://moodycamel.com/blog/2014/detailed-design-of-a-lock-free-queue)


