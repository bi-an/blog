---
title: 文件IO与进程
date: 2025-08-16 20:50:37
categories: 操作系统
tags:
    - 文件
    - 进程
---


## 打开文件的内核数据结构

注意：对同一个文件，不同进程拥有各自的文件表项，但是对每个文件，v节点表项在整个操作系统中只有一份。见下一节。

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/hG2hPTDM/image.png" alt="image"/></a>


### 文件描述符标志（即fd标志）：

目前只有一个，即 `close-on-exec`


### 文件状态标志：

| 文件状态标志      | 说明                           |
| ----------- | ---------------------------- |
| O\_RDONLY   | 只读打开                         |
| O\_WRONLY   | 只写打开                         |
| O\_RDWR     | 读、写打开                        |
| O\_EXEC     | 只执行打开                        |
| O\_SEARCH   | 只搜索打开目录                      |
| O\_APPEND   | 追加写                          |
| O\_NONBLOCK | 非阻塞模式                        |
| O\_SYNC     | 等待写完成（数据和属性）                 |
| O\_DSYNC    | 等待写完成（仅数据）                   |
| O\_RSYNC    | 同步读和写                        |
| O\_FSYNC    | 等待写完成（仅 FreeBSD 和 Mac OS X）  |
| O\_ASYNC    | 异步 I/O（仅 FreeBSD 和 Mac OS X） |



### i-node：包含以下内容

- 链接计数（指向该i节点的目录项数）；
- 文件类型、文件访问权限位、文件长度、指向文件数据块的指针等。`stat`结构中的大多数信息都取自i节点。
- 只要两项重要数据放在目录项中：文件名和i-node编号。

**注：**
- i-node中的链接称为“硬链接”，当硬链接数降为0时，才从磁盘的数据块中删除该文件，所以删除文件（即目录项）称为`unlink`，而不是`delete`。
- “软链接”：术语为“符号链接”，其数据块的实际内容是其指向的文件名字，i-node中的文件类型是`S_IFLINK`，表明是符号链接。

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/rFRyX7Rh/image.png" alt="磁盘、分区和文件系统"/></a>

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/3xM8K9g3/i.png" alt="i节点和数据块"/></a>



## 两个独立进程各自打开同一个文件：

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/hG2hPTDM/image.png" alt="两个独立进程各自打开同一个文件"/></a>

- O_APPEND：
如果使用 `O_APPEND` 标志打开一个文件，那么相应的标志也被设置到文件表项的文件状态标志中。每次对文件执行写操作时，文件表项中的当前文件偏移量首先会被设置为 i 节点表项中的文件长度（相对其他进程来说是原子操作）。这就使得每次写入的数据都追加到文件的当前尾端处。[这里](https://blog.csdn.net/yangbodong22011/article/details/63064166)有一个测试的例子，文章结论不见得正确，请参考评论的讨论。

以下是 `man page "write(2)"`：

> If the file was open(2)ed with O_APPEND, the file offset is first set to the end of the file before writing.  The adjustment of the file offset and the write operation are performed as an atomic step.

- lseek：
若一个文件用 `lseek` 定位到文件当前的尾端，则文件表项中的当前文件偏移量被设置为 i 节点表项中的当前文件长度（注意，此时，设置偏移量和写操作之间不是原子操作）。

---

## dup(1)后的内核数据结构：

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/s2mGQbcY/dup-1.png" alt="dup(1)"/></a>



### fork之后父进程和子进程对打开文件的共享：

<a href="https://postimages.org/" target="_blank"><img src="https://i.postimg.cc/TYbyqK0Y/fork.png" alt="fork"/></a>

- fork之后子进程复制父进程所有的打开描述符，并且保持其打开，即使执行了`exec()`，除非该文件描述符使用`fcntl()`设置了`FD_CLOEXEC`标志。



## 进程为什么会自动打开0, 1, 2三个文件描述符？

**答：**
- shell进程启动时，会自动打开这三个文件描述符（可能由配置项决定）；
- shell利用`fork()`开启用户进程（子进程），该子进程复制父进程shell的所有文件描述符，于是0, 1, 2文件描述符被打开；
- 由于子进程共享父进程的文件表项，子进程对文件状态标志（读、写、同步或非阻塞等）的修改，将会影响父进程。



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



### 测试结果：

#### 第一次运行：

```bash
$ ./a.out
read write
read write, append
```

#### 第二次运行：

```bash
$ ./a.out
read write, append
read write, append
```

**分析：**
- 第二次运行时，文件描述符0的初始状态保持了第一次运行的结果！
- 这是因为父进程shell的文件表项的文件状态标志被子进程`a.out`改变了。

#### 第三次运行：

重新启动shell，并运行`a.out`

```bash
$ ./a.out
read write
read write, append
```

**分析：**
- 第三次运行，结果与第一次一致，这说明我们的猜测正确。
- 父进程shell关闭之后，所有文件描述符被关闭，文件IO被关闭，文件表被释放。重启shell也就重置了文件表。



### 引申：
在此我们注意到，文件描述符0, 1, 2（标准输入、标准输出、标准错误）在一个shell及其所有子进程中，对应的文件（设备）是同一个。由于共享了文件表项，指向了同一个v-node表项，故都指向同一个虚拟终端。这与我们的平时观察一致，不然shell运行程序时，输入输出的入口在哪里呢？



## Linux 文件锁与记录锁

参考链接：[链接1](#)、[链接2](#)。



## 相关数据结构：

### `task_struct`

```c
struct task_struct {
  struct fs_struct* fs;
  struct files_struct* files;
};
```

### `files_struct`

```c
// File: <linux/fdtable.h>
struct files_struct {
    struct fdtable* fdt;
    struct file* fd_array[NR_OPEN_DEFAULT];
};
```

### `file`

```c
struct file {
  union {
    struct list_head fu_list;
    struct rcu_head fu_rcuhead;
  } f_u;
  struct path f_path;
  struct file_operations* f_op;
  spinlock_t f_lock;
  atomic_long_t f_count;
  unsigned int f_flags;
  fmode_t f_mode;
  loff_t f_pos;
  struct fown_struct f_owner;
  struct cred* f_cred;
  struct file_ra_state f_ra;
  u64 f_version;
  void* private_data;
  struct address_space* f_mapping;
  struct file* next;
  struct file* parent;
  const char* name;
  int lineno;
};
```


## 参考
- 《UNIX 环境高级编程》
- 《Linux 内核设计与实现（原书第 3 版） - Linux Kernel Development, Third Edition》，（美）拉芙（Love, R.）著；陈莉君，康华译. ——北京：机械工业出版社，2011.9（2021.5 重印）
