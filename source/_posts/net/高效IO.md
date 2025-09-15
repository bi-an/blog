---
title: 高效 IO
date: 2025-08-16 23:40:06
categories: 操作系统
tags: IO
---

## 高效的 IO 函数

- sendfile
- mmap + write
- splice
- readv

### 零拷贝 (Zero copy)

#### 文件到网络（最常见）

- sendfile(2)：一次系统调用把文件页直接从内核页缓存写到 socket（常用于 HTTP 文件静态服务）。
  - sendfile 通常能做到真正内核态零拷贝（文件页直接 DMA 到 NIC），但受限于协议（例如 SSL/TLS 会破坏）。
  - 输入必须是普通文件（regular file），支持 O_RDONLY 的磁盘文件或内核页缓存。
  - 不支持：管道、socket、设备文件（大多数特殊设备）、字符设备。
- splice(2) + vmsplice(2)：把数据在文件/pipe/socket 之间在内核内移动，避免用户态拷贝。可以做“文件 -> pipe -> socket”的零拷贝链路。
  - splice 输入/输出都可以是 pipe, socket, file（部分限制）。
- mmap + writev：把文件映射到用户地址空间（避免 read 系列复制），但仍会发生从用户到内核的复制（写时）。适合随机访问、共享映射场景。

| 特性         | sendfile           | splice                                   | vmsplice           |
| ------------ | ------------------ | ---------------------------------------- | ------------------ |
| 零拷贝       | ✅（文件 → socket） | ✅（文件/pipe/socket → 文件/pipe/socket） | ✅（用户页 → pipe） |
| 输入源       | 文件               | 文件 / pipe / socket                     | 用户缓冲区         |
| 输出目标     | Socket             | 文件 / pipe / socket                     | Pipe               |
| 用户空间参与 | 不参与             | 不参与                                   | 用户页作为数据源   |


```cpp
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    int pipefd[2];
    pipe(pipefd); // pipefd[1] 写端, pipefd[0] 读端

    char buf[] = "Hello zero-copy via vmsplice!";
    struct iovec iov;
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    // 用户缓冲区插入 pipe
    //
    // flags = SPLICE_F_GIFT ：
    // 内核“接管”了这些页（把页的所有权赠送给内核）。
    // 应用不能再修改 iov 指向的缓冲区，否则会产生数据竞争或内核数据损坏。
    // 这个模式能确保 真正零拷贝，但代价是用户缓冲区失去控制权。
    //
    // flags = 0
    // 在 vmsplice 返回后可以安全修改 iov 的内容
    // flags=0时，vmsplice默认采用SPLICE_F_MOVE尝试移动内存页，若失败则回退到拷贝；而write始终执行拷贝
    ssize_t n = vmsplice(pipefd[1], &iov, 1, 0);
    if (n < 0) { perror("vmsplice"); return 1; }

    // 从 pipe 读出
    char out[64];
    n = read(pipefd[0], out, sizeof(out));
    write(STDOUT_FILENO, out, n); // 输出到终端

    return 0;
}
```

#### 网络到网络 / 用户态网卡绕过

- AF_XDP / XDP / Netmap / PF_RING / DPDK：绕开传统 kernel network stack，把网卡队列直接映射到用户态缓冲区（NUMA/hugepages），实现用户态零拷贝和超低延迟。
- RDMA（InfiniBand / RoCE）：RDMA NIC 通过 DMA 直接把远端内存读写到本地内存，完全绕过 CPU 复制（需要硬件与协议支持）。

#### 加密 / TLS 场景

常规 TLS（OpenSSL）会在用户态做加密，破坏零拷贝。可用内核 TLS（KTLS）或 NIC TLS/offload 实现零拷贝出网（把加密放内核或网卡）。

#### 存储层

- O_DIRECT / direct I/O：绕过 page cache，用户空间 buffer 与磁盘 DMA 直接交互，但要求对齐（页/块对齐）。
- mmap + msync：对于某些场景可以减少复制（但写回仍会有开销）。

#### 异步 I/O 与零拷贝

io_uring（modern Linux）支持零拷贝模式（例如 splice/sendfile 的异步提交、SQE/ CQE），并且可以做更高效的批处理。

### Page cache 和异步 IO

https://www.xiaolincoding.com/os/8_network_system/zero_copy.html#%E5%A6%82%E4%BD%95%E5%AE%9E%E7%8E%B0%E9%9B%B6%E6%8B%B7%E8%B4%9D

- O_DIRECT: 绕过操作系统缓存，直接读写磁盘，可以避免缓存延迟，提高性能。可用于：
  - 数据库系统：对性能要求极高，且直接操作磁盘数据。
  - 存储设置：如 SSD、硬盘，直接与硬件设备进行高效的 I/O 操作。
  - 注意：由于绕过了缓存，所以 read 如果小于当前数据包的大小，则本次 read 后，内核会直接丢弃多余的数据。这是为了避免多余的数据在内存中驻留。

### 散布读写 (Scatter read/write)

- readv
- writev

散布读写支持一次性将数据从文件描述符读写到多个缓冲区：

- 避免多次系统调用；
- 直接分块读取，不需要额外用户态 memcpy 到不同的块。


## IO 复用

- select
- poll
- epoll
- 非阻塞 IO

如果 select / poll / epoll 通知可读写，那么一定可读写吗？
答案是不一定。因为内核不是 **实时地** 检查内核缓冲区是否有空间或有数据，所以内核的通知有时间差和虚假性。
而 epoll 等函数只关注事件变化，不检查缓冲区。这样可以提高效率。
最终的结果就是鼓励用户程序尝试，但是不保证一定成功，也就是可能阻塞。所以需要非阻塞 IO 来进一步提高性能。

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