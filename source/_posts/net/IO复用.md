---
title: IO复用
date: 2025-08-16 23:40:06
categories: 网络
tags:
    - 网络
    - 文件
---

## 高效的IO函数

- sendfile
- mmap + write
- splice
- readv

### 零拷贝 (Zero copy)

- mmap + write
- sendfile
- splice
- Page cache 和异步 IO

https://www.xiaolincoding.com/os/8_network_system/zero_copy.html#%E5%A6%82%E4%BD%95%E5%AE%9E%E7%8E%B0%E9%9B%B6%E6%8B%B7%E8%B4%9D



### 散布读写 (Scatter read/write)

- readv
- writev

散布读写支持一次性将数据从文件描述符读写到多个缓冲区：

- 避免多次系统调用；
- 直接分块读取，不需要额外用户态 memcpy 到不同的块。


## IO复用

- select
- poll
- epoll

### epoll

- 减少数据拷贝：select / poll 只有一个函数，这会要求每次调用都必须将描述事件的数据从用户空间复制到内核空间；所以 epoll 拆分成三个函数，用户可以向内核直接注册事件数据；
- 红黑树：epoll 事件数据是用红黑树来记录，增删查改的时间复杂度为 O(logn) ；select / poll 是线性扫描，时间复杂度 O(n) 。红黑树需要额外的空间，所以这是空间换时间的办法。

## 参考

https://blog.csdn.net/salmonwilliam/article/details/112347938