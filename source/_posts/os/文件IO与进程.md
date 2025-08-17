---
title: 文件IO与进程
date: 2025-08-16 20:50:37
categories: 操作系统
tags:
    - 文件
    - 进程
---


## 进程打开文件

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/50jyHyjJ/image.png" alt="image"/></a>

- `struct task_struct` (进程控制块)
  - 每个进程有一个 `task_struct`结构体，内核用它来描述进程。
  - 里面有一个指针 `files` ，执行该进程的 `files_struct` 。

```c
// https://elixir.bootlin.com/linux/v6.16/source/include/linux/sched.h

/* 进程控制块 */
struct task_struct {
    struct files_struct *files; // 进程当前打开的文件
    // ...
};
```

- `files_struct` (进程的文件表)
  - 这里保存了一个指向 fd 数组 的指针。
  - fd 数组的下标就是 0, 1, 2...，每个元素指向一个 `file *` 结构体。

```cpp
// https://elixir.bootlin.com/linux/v6.16/source/include/linux/fdtable.h

struct files_struct {
    struct fdtable __rcu *fdt;                          // fd 表（动态管理）
    struct file __rcu    *fd_array[NR_OPEN_DEFAULT];    // 固定大小数组（早期 fd 数组）
                                                        // NR_OPEN_DEFAULT 通常为 1024
                                                        // 早期没有设置 FD_CLOEXEC 新特性
    // 为什么 fdt 与 fd_array 同时存在？
    // 1. 性能优化：大多数程序，fd 都在 0~1023，直接访问数组更快
    // 2. 向后兼容：早期接口会访问fd_array
    // 3. 动态扩展：如果 fd 超过 NR_OPEN_DEFAULT，内核会复制 fd_array 到 fdt->fd 来保证一致性。

    // ... `struct files_struct' 还有其他成员
};

// fd 表（动态管理）
struct fdtable {
    unsigned int        max_fds;
    struct file __rcu **fd;             /* 当前打开的 fd 指针数组 */
    unsigned long      *close_on_exec;  // fd 标志，目前只有一个
                                        // FD_CLOEXEC 定义在 <fcntl.h> 中
                                        // close_on_exec 指向一个位图(bitmap)，每个bit代表一个fd
                                        // 如果bit=1，表示该fd设置了FD_CLOEXEC
    unsigned long      *open_fds;       // 标记哪些fd是打开的，也是位图
    unsigned long      *full_fds_bits;  // 辅助位图，用于快速找到空闲fd
    struct rcu_head     rcu;
};
```

- `struct file` (打开文件表项)
  - 内核为每次 `open()`、`pipe()`、`socket()` 创建一个 `struct file`。
  - 它记录了文件状态（读写偏移、flag、引用计数等）。

```c
// https://elixir.bootlin.com/linux/v6.16/source/include/linux/fs.h

struct file {
    spinlock_t                    f_lock;
    fmode_t                       f_mode;
    const struct file_operations *f_op;
    struct address_space         *f_mapping;
    void                         *private_data;  // 比如 socket
    struct inode                 *f_inode;
    unsigned int                  f_flags;  // 文件状态标志，如 O_RDONLY, O_NONBLOCK, O_APPEND 等
    unsigned int                  f_iocb_flags;
    const struct cred            *f_cred;
    struct fown_struct           *f_owner;
    /* --- cacheline 1 boundary (64 bytes) --- */
    struct path f_path;
    union {
        /* regular files (with FMODE_ATOMIC_POS) and directories */
        struct mutex f_pos_lock;
        /* pipes */
        u64 f_pipe;  // 管道
    };
    // ...
};
```

- inode / pipe / socket 内核对象
  - `struct file` 再指向更底层的对象，比如 inode（磁盘文件）、socket 缓冲区、pipe 缓冲区。

  - i-node 包含以下内容
    - 链接计数（指向该i节点的目录项数）；
    - 文件类型、文件访问权限位、文件长度、指向文件数据块的指针等。`stat`结构中的大多数信息都取自i节点。
    - 只有两项重要数据放在目录项中：文件名和i-node编号。

<div style="text-align: center">
<figure>
  <img src="https://i.postimg.cc/rFRyX7Rh/image.png" alt="磁盘、分区和文件系统">
  <figcaption>磁盘、分区和文件系统</figcaption>
</figure>
</div>

<div style="text-align: center">
<figure>
  <img src="https://i.postimg.cc/3xM8K9g3/i.png" alt="i节点和数据块">
  <figcaption>i节点和数据块</figcaption>
</figure>
</div>


软链接与硬链接

| 类型                                | 定义                                       |
| --------------------------------- | ---------------------------------------- |
| **硬链接 (Hard Link)**               | 文件系统中对同一个 **inode** 的不同名字引用。它们指向同一个文件内容。 |
| **软链接 (Symbolic Link / Symlink)** | 类似快捷方式，是一个 **独立文件**，内容是指向目标文件的路径。        |

- 硬链接：当硬链接数降为0时，才从磁盘的数据块中删除该文件，所以删除文件（即目录项）称为`unlink`，而不是`delete`。
- 软链接：i-node中的文件类型是`S_IFLINK`，表明是符号链接。

| 特性         | 硬链接                  | 软链接                        |
| ---------- | -------------------- | -------------------------- |
| 是否指向 inode | 是，直接指向同一 inode       | 否，指向目标路径                   |
| 是否可以跨文件系统  | 否，只能在同一分区            | 可以跨分区                      |
| 是否可以链接目录   | 通常不能（除非 root）        | 可以                         |
| 删除目标文件后   | 文件内容仍可访问             | 链接会失效（称为“悬挂链接”）            |
| 占用空间       | 不占用额外数据空间（只是多了一个目录项） | 占用少量空间存储路径信息               |
| 更新文件内容     | 所有硬链接同步可见            | 通过软链接修改目标文件内容时可见，软链接本身只是路径 |



## 两个独立进程各自打开同一个文件

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/hG2hPTDM/image.png" alt="两个独立进程各自打开同一个文件"/></a>

- O_APPEND：
  - 原子操作：如果使用 `O_APPEND` 标志打开一个文件，那么相应的标志也被设置到文件表项的文件状态标志中。每次对文件执行写操作时，文件表项中的当前文件偏移量首先会被设置为 i 节点表项中的文件长度（相对其他进程来说是**原子**操作，不论是两个独立的进程，还是父子进程）。这就使得每次写入的数据都追加到文件的当前尾端处。[这里](https://blog.csdn.net/yangbodong22011/article/details/63064166)有一个测试的例子，文章结论不见得正确，请参考评论的讨论。
  - `PIPE_BUF`：只保证小于`PIPE_BUF`的内容是原子；如果大于则可能被多次多段写入。PIPE_BUF 是管道（pipe）单次写入保证原子的最大字节数，Linux 上是 4096 字节。

```bash
# 查看 PIPE_BUF 大小
# `/tmp' 可以换成任意文件系统路径
$ getconf PIPE_BUF /tmp
4096
# 也可以查看所有文件系统相关的 PIPE_BUF 限制
$ getconf -a PIPE_BUF
```

  以下是 `man 2 write` 关于 `O_APPEND` 的说明：

> If the file was open(2)ed with O_APPEND, the file offset is first set to the end of the file before writing.  The adjustment of the file offset and the write operation are performed as an atomic step.

- lseek：

若一个文件用 `lseek` 定位到文件当前的尾端，则文件表项中的当前文件偏移量被设置为 i 节点表项中的当前文件长度（注意，此时，设置偏移量和写操作之间不是原子操作）。

---

## `dup()`后的内核数据结构

`dup()` / `dup2()` 只复制 fd ，也就是在 fd 数组中新增了一个 fd 项。一般用来重定向。

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/s2mGQbcY/dup-1.png" alt="dup(1)"/></a>

---

## fork与文件共享

- 进程每打开一个文件，都会新建一个 `struct file` ，并添加到 fd 数组或 fd 表中。
  - 对同一个文件，不同进程拥有各自的文件表项。
  - 但是对每个文件，v节点表项在整个操作系统中只有一份。
- `fork()` 后的子进程直接复制父进程的 fd 数组，`exec()` 也不能将其替换；
  - 子进程对 `struct task_struct` 是深拷贝，所以 fd 数组被复制；
  - 但是子进程对 fd 数组是浅拷贝，fd 数组中的 `struct file*` 仍然指向父进程创建的 `struct file ` （共享）；
  - 所以子进程共享了文件状态标志 (O_APPEND, O_NONBLOCK, O_RDONLY 等)、当前文件偏移量。
- 除非该文件描述符使用`fcntl()`设置了`FD_CLOEXEC`标志，此时 `exec` 会关闭继承的文件描述符。

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/TYbyqK0Y/fork.png" alt="fork"/></a>


**子进程对文件表项的修改，会不会影响父进程？**

- shell进程启动时，会自动打开这三个文件描述符（可能由配置项决定）；
- shell利用`fork()`开启用户进程（子进程），该子进程复制父进程shell的所有文件描述符，于是0, 1, 2文件描述符被打开；
- 由于子进程**共享**父进程的**文件表项**，子进程对文件状态标志（读、写、同步或非阻塞等）、文件偏移量的修改，将**会影响父进程**。


测试代码：

```c
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define err_sys(x)                                                             \
  {                                                                            \
    perror(x);                                                                 \
    exit(1);                                                                   \
  }

void pr_fl(int fd);             // 自定义函数：打印文件状态标志
void set_fl(int fd, int flags); // 自定义函数：设置文件状态标志

int main() {
  pr_fl(0);

  set_fl(0, O_APPEND);
  pr_fl(0);

  return 0;
}

void set_fl(int fd, int flags) {
  int val;

  if ((val = fcntl(fd, F_GETFL, 0)) < 0)
    err_sys("fcntl F_GETFL error");

  val |= flags;

  if (fcntl(fd, F_SETFL, val) < 0)
    err_sys("fcntl F_SETFL error");
}

void pr_fl(int fd) {
  int val;

  // do not guarantee success on certain system, check EINVAL first
  if ((val = fcntl(fd, F_GETFL, 0)) < 0)
    err_sys("fcntl F_GETFL error");

  switch (val & O_ACCMODE) {
  case O_RDONLY:
    printf("read only");
    break;
  case O_WRONLY:
    printf("write only");
    break;
  case O_RDWR:
    printf("read write");
    break;
  default:
    err_sys("unknown open type");
  }

  if (val & O_CREAT)
    printf(", create");
  if (val & O_APPEND)
    printf(", append");
  if (val & O_NONBLOCK)
    printf(", non-block");
  if (val & O_SYNC)
    printf(", synchronized file");
  // if (val & O_DSYNC)
  //   printf(", synchronize data");

  putchar('\n');
}
```

- 第一次运行：

```bash
$ ./a.out
read write
read write, append
```

- 第二次运行：

```bash
$ ./a.out
read write, append
read write, append
```

  - 分析
    - 第二次运行时，文件描述符0的初始状态保持了第一次运行的结果！
    - 这是因为父进程shell的文件表项的文件状态标志被子进程`a.out`改变了。

- 第三次运行：

  重新启动shell，并运行`a.out`

```bash
$ ./a.out
read write
read write, append
```

  - 分析
    - 第三次运行，结果与第一次一致，这说明我们的猜测正确。
    - 父进程shell关闭之后，所有文件描述符被关闭，文件IO被关闭，文件表被释放。重启shell也就重置了文件表。

  - 引申：
    在此我们注意到，文件描述符0, 1, 2（标准输入、标准输出、标准错误）在一个shell及其所有子进程中，对应的文件（设备）是同一个。由于共享了文件表项，指向了同一个v-node表项，故都指向同一个虚拟终端。这与我们的平时观察一致，不然shell运行程序时，输入输出的入口在哪里呢？

### 管道

通过上文的叙述，我们很容易想到管道本质上也是一种特殊的文件，所以管道机制之所以可以进程间通信也是根据共享文件表项保证的。
管道和文件进行进程间通信的本质相同。

## Linux 文件锁与记录锁

TODO

参考链接：[链接1](https://www.cnblogs.com/xuyh/p/3278881.html)、[链接2](https://www.cnblogs.com/fortunely/p/15219611.html)。


## 参考
- 《UNIX 环境高级编程》
- 《Linux 内核设计与实现（原书第 3 版） - Linux Kernel Development, Third Edition》，（美）拉芙（Love, R.）著；陈莉君，康华译. ——北京：机械工业出版社，2011.9（2021.5 重印）
- [图解进程控制块stask_struct](https://github.com/antsHub/task_struct/tree/main)
