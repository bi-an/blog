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


### Page cache 和异步 IO

https://www.xiaolincoding.com/os/8_network_system/zero_copy.html#%E5%A6%82%E4%BD%95%E5%AE%9E%E7%8E%B0%E9%9B%B6%E6%8B%B7%E8%B4%9D

- O_DIRECT: 绕过操作系统缓存，直接读写磁盘，可以避免缓存延迟，提高性能。可用于：
  - 数据库系统：对性能要求极高，且直接操作磁盘数据。
  - 存储设置：如 SSD、硬盘，直接与硬件设备进行高效的 I/O 操作。
  - 注意：由于绕过了缓存，所以read如果小于当前数据包的大小，则本次read后，内核会直接丢弃多余的数据。这是为了避免多余的数据在内存中驻留。

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
- 非阻塞IO

如果 select / poll / epoll 通知可读写，那么一定可读写吗？
答案是不一定。因为内核不是**实时地**检查内核缓冲区是否有空间或有数据，所以内核的通知有时间差和虚假性。
而 epoll 等函数只关注事件变化，不检查缓冲区。这样可以提高效率。
最终的结果就是鼓励用户程序尝试，但是不保证一定成功，也就是可能阻塞。所以需要非阻塞IO来进一步提高性能。

### epoll

- 减少数据拷贝：select / poll 只有一个函数，这会要求每次调用都必须将描述事件的数据从用户空间复制到内核空间；所以 epoll 拆分成三个函数，用户可以向内核直接注册事件数据；
- 红黑树：epoll 事件数据是用红黑树来记录，增删查改的时间复杂度为 O(logn) ；select / poll 是线性扫描，时间复杂度 O(n) 。红黑树需要额外的空间，所以这是空间换时间的办法。

#### `EPOLLONESHOT`

阅读 manual：

> Since  even  with  edge-triggered  epoll,  multiple events can be generated upon receipt of multiple chunks of data, the caller has the option to specify the EPOLLONESHOT flag, to tell epoll to disable the associated file descriptor after the receipt of an event  with  epoll_wait(2).   When the EPOLLONESHOT flag is specified, it is the caller's responsibility to rearm the file descriptor using epoll_ctl(2) with EPOLL_CTL_MOD.

如果某个文件描述符上有多个数据块到达，那么即使是边沿触发也无法保证事件只通知一次。这可能是由于数据包过大被分片，或者是新数据到达。
- 这在单线程程序上不会有太大影响，因为对同一个 fd 不会造成重复读写。
- 多线程程序中，fd 准备好后，我们常常将这个 fd 交给某个线程去处理。此时如果 fd 有新的事件，会造成多线程处理同一个 fd 的情况。
  - 为了避免竞争，要么加锁；要么使用内核的 ONESHOT 机制。后者由内核保证，无锁，更高效。
- `EPOLLONSHOT` 需要调用者自行 reset 这个标志。


## 参考

https://blog.csdn.net/salmonwilliam/article/details/112347938