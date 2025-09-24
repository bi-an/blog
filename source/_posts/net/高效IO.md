---
title: 高效 IO
date: 2025-08-16 23:40:06
categories: 操作系统
tags: IO
---

## 高效的 IO 函数

- `sendfile`
- `mmap` + `write`
- `splice`
- `readv` / `writev`：一次性读写多个缓冲区，减少系统调用 `read`/`write`的次数。

### 零拷贝 (Zero copy)

#### 文件到网络（最常见）

参考
：[玩转 Linux 内核：超硬核，基于 mmap 和零拷贝实现高效的内存共享](https://zhuanlan.zhihu.com/p/642866437)

##### `read` + `write`

从磁盘读取文件，发送到网络：

1. `read`:
   1. DMA 拷贝：磁盘 → 内核页缓存；
   2. CPU 拷贝：内核页缓存 → 用户缓冲区。
2. `write`:
   1. CPU 拷贝：用户缓冲区 → 内核 socket 缓冲区；
   2. DMA 拷贝：内核 socket 缓冲区 → 网卡。

发生了 4 次拷贝、4 次上下文切换（每次系统调用都会发生两次上下文切换：用户态 → 内核态，内核态 → 用户
态）。

#### `mmap` + `write`

`mmap` 的本质：虚拟地址（用户空间） → 物理页

将一段 用户空间的虚拟地址空间 映射到某个物理资源（文件、设备、或匿名页）上：

|        mmap 类型         |                   说明                   |
| :----------------------: | :--------------------------------------: |
|         映射文件         | 映射到关联文件的内核页缓存（page cache） |
|     匿名内存（RAM）      |               映射到物理页               |
| 映射设备（如 GPU、FPGA） |              映射到设备地址              |

1. `mmap`：
   1. DMA 拷贝：磁盘 → 内核页缓存（同时也是用户空间可见的）；
2. `write`:
   1. CPU 拷贝：内核页缓存 → 内核 socket 缓冲区；
   2. DMA 拷贝：内核 socket 缓冲区 → 网卡。

发生了 3 次拷贝、4 次上下文切换。

##### `sendfile`

函数签名：

```c
ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
```

一次系统调用 `sendfile` 就可以完成从 磁盘文件 → 网卡 的拷贝。

1. DMA 拷贝：磁盘 → 内核磁盘缓冲区；
2. CPU 拷贝：内核磁盘缓冲区 → 内核 socket 缓冲区；
3. DMA 拷贝：内核 socket 缓冲区 → 网卡。

发生了 3 次拷贝、2 次上下文切换。

注意到，`sendfile` 方法 IO 数据对用户空间完全不可见，所以只能适用于完全不需要用户空间处理的情况，比
如静态文件服务器。

`sendfile` 被称为“零拷贝”，因为完全绕过用户空间，没有用户空间和内核空间的数据拷贝。

- `mmap` + `write` 只是多了一次系统调用（多了 2 次上下文切换），但是一般不称为零拷贝，因为它本质上是
  把文件内容映射到用户空间，并且数据传输还是要 `write` 来完成。
- `io_uring` 是异步 IO，核心机制之一就是通过 `mmap` 映射共享环形缓冲区。
- `mmap` 是共享内存的方式之一，以公用文件名中转。文件名一般使用 `/dev/shm/` 下的文件名，因为其为
  `tmpfs` 内存文件系统，数据不会落盘；如果使用普通文件名，则可能触发磁盘读写，尽管由于页缓存的存在，
  除非缺页异常，否则被共享者直接读取页缓存就可以，但是终究有实际磁盘 IO 的参与，效率不高。

##### `splice`

函数签名：

```c
ssize_t splice(int fd_in, loff_t *off_in,
               int fd_out, loff_t *off_out,
               size_t len, unsigned int flags);
```

`splice` 把数据从一个文件描述符“拼接”到另一个文件描述符，而其中至少一个必须是管道（pipe）。

为什么一定要 pipe 参与？

- pipe 是内核态缓冲区 pipe 是一种特殊的内核对象，拥有自己的 buffer（pipe buffer）。、

  - splice 利用 pipe buffer 来暂存数据，避免用户空间拷贝。
  - pipe buffer 是通用 buffer ，是隔离的、一次性消费的，适合做数据通路。

- 普通文件或 socket 虽然也有缓冲区（页缓存、socket buffer），但是它们不属于通用的缓冲区：

  - socket buffer 的数据必须经过 TCP/IP 协议栈处理（如分片、拥塞控制、校验等）。
  - 页缓存属于文件系统，且可能被多进程共享（COW，写时复制）。

示例：

```c
int pipefd[2];
pipe(pipefd);

// 文件 → pipe
splice(file_fd, NULL, pipefd[1], NULL, len, SPLICE_F_MOVE);

// pipe → socket
splice(pipefd[0], NULL, socket_fd, NULL, len, SPLICE_F_MOVE);
```

##### `vmsplice`

函数签名：

```c
ssize_t vmsplice(int fd, const struct iovec *iov,
                 unsigned long nr_segs, unsigned int flags);
```

`vmsplice` 将用户空间的内存页映射到 pipe buffer 。

示例：

```c
struct iovec iov = { .iov_base = user_buf, .iov_len = len };
vmsplice(pipefd[1], &iov, 1, SPLICE_F_GIFT);
splice(pipefd[0], NULL, socket_fd, NULL, len, 0);
```

`vmsplice` 几乎总是需要与 `splice` 协作使用，因为 `vmsplice` 本身只能将用户空间的内存页映射到一个
pipe 中，而不能直接发送到 socket 或写入文件。

| 特性         | sendfile            | splice                                    | vmsplice            |
| ------------ | ------------------- | ----------------------------------------- | ------------------- |
| 零拷贝       | ✅（文件 → socket） | ✅（文件/pipe/socket → 文件/pipe/socket） | ✅（用户页 → pipe） |
| 输入源       | 文件                | 文件 / pipe / socket                      | 用户缓冲区          |
| 输出目标     | Socket              | 文件 / pipe / socket                      | Pipe                |
| 用户空间参与 | 不参与              | 不参与                                    | 用户页作为数据源    |

完整示例：

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

- AF_XDP / XDP / Netmap / PF_RING / DPDK：绕开传统 kernel network stack，把网卡队列直接映射到用户态
  缓冲区（NUMA/hugepages），实现用户态零拷贝和超低延迟。
- RDMA（InfiniBand / RoCE）：RDMA NIC 通过 DMA 直接把远端内存读写到本地内存，完全绕过 CPU 复制（需要
  硬件与协议支持）。

#### 加密 / TLS 场景

常规 TLS（OpenSSL）会在用户态做加密，破坏零拷贝。可用内核 TLS（KTLS）或 NIC TLS/offload 实现零拷贝出
网（把加密放内核或网卡）。

#### 存储层

- O_DIRECT / direct I/O：绕过 page cache，用户空间 buffer 与磁盘 DMA 直接交互，但要求对齐（页/块对齐
  ）。
- mmap + msync：对于某些场景可以减少复制（但写回仍会有开销）。

#### 异步 I/O 与零拷贝

io_uring（modern Linux）支持零拷贝模式（例如 splice/sendfile 的异步提交、SQE/ CQE），并且可以做更高
效的批处理。

### Page cache 和异步 IO

https://www.xiaolincoding.com/os/8_network_system/zero_copy.html#%E5%A6%82%E4%BD%95%E5%AE%9E%E7%8E%B0%E9%9B%B6%E6%8B%B7%E8%B4%9D

- O_DIRECT: 绕过操作系统缓存，直接读写磁盘，可以避免缓存延迟，提高性能。可用于：
  - 数据库系统：对性能要求极高，且直接操作磁盘数据。
  - 存储设置：如 SSD、硬盘，直接与硬件设备进行高效的 I/O 操作。
  - 注意：由于绕过了缓存，所以 read 如果小于当前数据包的大小，则本次 read 后，内核会直接丢弃多余的数
    据。这是为了避免多余的数据在内存中驻留。

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

如果 select / poll / epoll 通知可读写，那么一定可读写吗？答案是不一定。因为内核不是 **实时地** 检查
内核缓冲区是否有空间或有数据，所以内核的通知有时间差和虚假性。而 epoll 等函数只关注事件变化，不检查
缓冲区。这样可以提高效率。最终的结果就是鼓励用户程序尝试，但是不保证一定成功，也就是可能阻塞。所以需
要非阻塞 IO 来进一步提高性能。

### epoll

- 减少数据拷贝：select / poll 只有一个函数，这会要求每次调用都必须将描述事件的数据从用户空间复制到内
  核空间；所以 epoll 拆分成三个函数，用户可以向内核直接注册事件数据；
- 红黑树：epoll 事件数据是用红黑树来记录，增删查改的时间复杂度为 O(logn) ；select / poll 是线性扫描
  ，时间复杂度 O(n) 。红黑树需要额外的空间，所以这是空间换时间的办法。

#### `EPOLLONESHOT`

阅读 manual：

> Since even with edge-triggered epoll, multiple events can be generated upon receipt of multiple
> chunks of data, the caller has the option to specify the EPOLLONESHOT flag, to tell epoll to
> disable the associated file descriptor after the receipt of an event with epoll_wait(2). When the
> EPOLLONESHOT flag is specified, it is the caller's responsibility to rearm the file descriptor
> using epoll_ctl(2) with EPOLL_CTL_MOD.

如果某个文件描述符上有多个数据块到达，那么即使是边沿触发也无法保证事件只通知一次。这可能是由于数据包
过大被分片，或者是新数据到达。

- 这在单线程程序上不会有太大影响，因为对同一个 fd 不会造成重复读写。
- 多线程程序中，fd 准备好后，我们常常将这个 fd 交给某个线程去处理。此时如果 fd 有新的事件，会造成多
  线程处理同一个 fd 的情况。
  - 为了避免竞争，要么加锁；要么使用内核的 ONESHOT 机制。后者由内核保证，无锁，更高效。
- `EPOLLONSHOT` 需要调用者自行 reset 这个标志。

## 参考

https://blog.csdn.net/salmonwilliam/article/details/112347938
