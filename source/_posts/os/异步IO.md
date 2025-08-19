---
title: 异步IO
date: 2025-08-19 20:26:00
categories: 操作系统
tags: IO
---

## IO模型概念

IO 模型通常按两条维度划分：

1. 阻塞 vs 非阻塞

   - 阻塞 IO（Blocking IO）：调用 read/recv 等函数时，如果数据没准备好，进程会被挂起，直到数据就绪。
   - 非阻塞 IO（Non-blocking IO）：调用 read/recv 时，如果数据没准备好，直接返回 EAGAIN 或 EWOULDBLOCK，进程继续做别的事情。

2. 同步 vs 异步

   - 同步（Synchronous）：调用者要等待操作完成才能继续。
     - 阻塞 IO + 同步：最常见，比如普通 read(fd, buf, n)
     - 非阻塞 IO + 同步：调用立即返回，如果没数据则报错或返回 0

   - 异步（Asynchronous）：调用者发起操作后，不需要等待，操作完成时通过回调、信号、事件通知等告知结果。

⚡ 关键：异步 IO 的核心是不阻塞当前线程，而结果通知是通过事件或回调完成的。


## Linux 常见异步 IO 方式

Linux 下主要有四种机制：

1. POSIX AIO（aio_* 系列）

   - 系统调用：aio_read(), aio_write()

   - 完成通知方式：
     - 轮询 aio_error()
     - 信号通知 SIGIO
     - 回调函数 sigevent.sigev_notify = SIGEV_THREAD

   - 使用场景：文件 IO，可以在后台发起读写请求，主线程继续工作。
   - ⚠️ 目前性能不如 epoll + 线程池模拟异步。

2. 信号驱动 IO（SIGIO）

   - 进程或文件描述符注册 F_SETOWN，开启 O_ASYNC
   - 当 fd 可读写时，内核发信号给进程
   - 通常用于少量 fd 的异步事件

3. I/O 多路复用（select, poll, epoll）

   - 本质是非阻塞 + 事件通知
   - Epoll + 非阻塞 IO 可以模拟高效的异步 IO
   - 适合网络服务器、socket 编程

   - 典型流程：
     - 设置 fd 为非阻塞（否则 read/write 可能阻塞，因为 epoll 本质是同步的）
     - 注册 fd 到 epoll，关注 EPOLLIN / EPOLLOUT
     - 调用 epoll_wait 等待事件
     - 事件触发时读取或写入数据

4. Linux AIO（io_uring）

   - 新一代高性能异步 IO 接口
   - 支持文件、网络 IO
   - 提供 提交队列 + 完成队列，几乎零系统调用开销
   - 可以真正做到线程几乎不阻塞等待

### 异步 IO 的优点

- 不阻塞主线程，提高吞吐量
- 可同时处理大量 IO（特别是网络/文件服务器）
- 与多线程相比，降低线程上下文切换开销

### 异步 IO 的缺点

- 编程复杂度高（需要事件驱动、回调或状态机）
- 错误处理和信号安全问题复杂
- 文件异步 IO 性能在传统 AIO 下不一定比多线程高

### Linux 下常见异步 I/O 机制对比

| 特性 / 机制     | POSIX AIO              | epoll + 非阻塞 IO           | io\_uring          | 信号驱动 IO (SIGIO)         |
| ----------- | ---------------------- | ------------------------ | ------------------ | ----------------------- |
| **类型**      | 异步文件 IO                | 多路复用 + 非阻塞网络 IO          | 高性能异步 IO           | 异步事件通知                  |
| **支持对象**    | 文件                     | 文件描述符（socket、管道等）        | 文件 + 网络 + 其他 IO    | 文件描述符（socket、pipe）      |
| **用户态/内核态** | 系统调用提交，内核异步处理          | 用户态轮询/等待事件，内核检查 fd       | 用户态 SQ + 内核 CQ     | 用户注册 fd，内核通过信号通知        |
| **提交方式**    | aio\_read/aio\_write   | 写入 fd 并通过 epoll\_wait 检查 | 写入 SQ（批量可提交）       | 设置 O\_ASYNC + F\_SETOWN |
| **完成通知**    | 信号 / 回调 / aio\_error轮询 | epoll\_wait 返回就绪事件       | 完成队列 (CQ)，阻塞或非阻塞读取 | 信号处理函数 (SIGIO)          |
| **性能**      | 中等，系统调用多               | 高，单线程处理大量 fd             | 很高，几乎零系统调用，批量提交    | 较低，信号开销大，适合少量 fd        |
| **编程复杂度**   | 中等偏复杂                  | 中等，需要状态机处理               | 高，但灵活，可批量和链式操作     | 高，信号处理函数限制多，必须信号安全      |
| **适合场景**    | 文件异步读写                 | 高并发网络服务器                 | 高性能文件和网络 IO        | 少量异步事件或控制信号触发场景         |


## Linux io_uring

io_uring 是 Linux 内核自 5.1 版本引入的一个异步 I/O 框架，它提供了 低延迟、高吞吐的异步文件和网络 I/O。它的特点是：

- 零拷贝提交：应用程序可以直接向内核提交 I/O 请求，无需系统调用每次阻塞。
- 环形队列机制：通过共享内存的 提交队列（Submission Queue, SQ） 和 完成队列（Completion Queue, CQ），用户态和内核态可以高效交互。
- 支持多种 I/O 类型：文件读写、网络收发、文件同步、缓冲区操作等。
- 批量提交和完成：可以一次提交多个 I/O 请求，并批量获取完成结果。

简单理解：它把传统阻塞 I/O 的 “系统调用来回” 改成了 共享环形队列 + 异步通知。

### 安装

1. 方法一：从 APT 安装

```bash
sudo apt update
sudo apt install liburing-dev
```

检查安装路径

```bash
ls /usr/include/liburing.h
```

2. 方法二：从源码安装

```bash
git clone https://github.com/axboe/liburing.git
cd liburing
make
sudo make install
```

### io_uring 的核心数据结构

Submission Queue（SQ）

- 用户态将 I/O 请求放入 SQ。
- SQ 是一个环形数组，存放 io_uring_sqe（I/O 请求条目）。
- 用户通过 系统调用 io_uring_enter 将 SQ 中的新请求通知内核。
- 内核会按顺序处理 SQ 中的 I/O 请求。

| 字段       | 作用                                        |
| -------- | ----------------------------------------- |
| `opcode` | I/O 类型，如读、写、fsync、accept、sendmsg          |
| `fd`     | 文件描述符                                     |
| `off`    | 偏移量（文件 I/O）                               |
| `addr`   | 用户缓冲区地址                                   |
| `len`    | I/O 数据长度                                  |
| `flags`  | 请求标志，如 `IOSQE_FIXED_FILE`、`IOSQE_IO_LINK` |


Completion Queue（CQ）

- 内核完成 I/O 后，将结果写入 CQ。
- CQ 也是一个环形数组，存放 io_uring_cqe（完成条目）。
- 用户可以轮询或等待 CQ 获取完成结果。

| 字段          | 作用                          |
| ----------- | --------------------------- |
| `res`       | I/O 结果，成功为正数（读写字节数），失败为负错误码 |
| `user_data` | 用户自定义数据，方便识别请求              |


### io_uring 工作流程

```
+-----------+          +-----------+
| User App  | <----->  |  Kernel  |
+-----------+          +-----------+
      |                     |
      |  write SQE to SQ    | <- Submission Queue
      |-------------------->|
      |                     |
      |   io_uring_enter    | <- 通知内核处理
      |-------------------->|
      |                     |
      | <------------------ | <- CQE 放入 CQ
      |  read CQE from CQ   |
```

- 用户态填充 SQE（Submission Queue Entry）。
- 调用 io_uring_enter() 提交 SQE （不阻塞）。
- 内核处理 I/O 请求。
- 内核把完成结果写入 CQ。
- 用户态可以：
  - 轮询 CQ：主动读取 CQE（Completion Queue Entry）
  - 注册回调（liburing 新版本支持 IORING_SETUP_IOPOLL + IORING_SETUP_SQPOLL 或自己封装）

注意：

- 异步 I/O ≠ 必须用回调。关键是提交后不阻塞等待，可以同步轮询完成结果，也可以异步触发回调。
- 回调是一种可选的使用方式。
- 最核心的是 共享环形队列 + 完成队列，用户可以同步取结果也可以异步通知。

### 为什么 io_uring 没有强制回调

传统异步 I/O（比如 Windows IOCP）必须注册回调或事件句柄，因为内核不会给你“主动通知”。

Linux io_uring 的设计哲学是：

- 用户态和内核共享内存 → 用户态可以自己轮询完成队列。
- 减少系统调用次数 → 不依赖信号或回调触发。
- 需要回调时，用户可以自己封装一个事件循环。

所以你看到 io_uring 的官方示例都是 顺序写代码，但是仍然是异步 I/O，因为：

- 提交后内核可以并行处理多个 I/O。
- 用户态无需阻塞等待内核完成处理（可以去做别的事）。

### io_uring 的使用示例（C 语言）

{% include_code lang:cpp io/io_uring_hello.c %}

这个例子展示了 最基本的异步文件读取：

- 初始化 ring。
- 获取一个 SQE 并填充读请求。
- 提交 SQE。
- 等待 CQE 获取结果。
- 标记完成并清理。
