---
title: C++ 调试工具
date: 2024-01-23 21:27:05
categories: c/cpp
tags: debug
---

## 单元测试框架

[CppTest](https://cpptest.sourceforge.io/tutorial.html)

## gdb

```gdb
(gdb) breakpoint exit
(gdb) breakpoint _exit
(gdb) breakpoint atexit
(gdb) breakpoint abort
```

Enable coredump:
[how to do](https://medium.com/@sourabhedake/core-dumps-how-to-enable-them-73856a437711)

```bash
ulimit -c unlimited
```

Where is the core dumped file:

```bash
grep 'kernel.core_pattern' /etc/sysctl.conf
```

## strace

Example:

```bash
strace -f -o strace.log -tt -y -yy -e trace=desc,process,network
```

Refer to [here](https://gist.github.com/graste/929bb122c353bdd90c20)

> -e trace=ipc – communication between processes (IPC) -e trace=memory – memory syscalls -e
> trace=network – network syscalls -e trace=process – process calls (like fork, exec) -e
> trace=signal – process signal handling (like HUP, exit) -e trace=file – file related syscalls -e
> trace=desc – all file descriptor related system calls

## DDT (Distributed Debugging Tool, Arm Forge)

- 来源：最初由 Allinea 开发，后来并入 Arm Forge 工具链。
- 用途：面向 大规模并行和分布式应用的调试器，广泛用于 HPC (高性能计算)。
- 工作方式：
  - 能 attach 到数千个 MPI 进程或 OpenMP 线程，统一调试。
  - 支持断点、变量检查、内存错误检测。
  - 与性能分析工具（如 Arm MAP）结合使用，可以同时做调试和性能分析。
- 适用场景：超级计算机上的 MPI/OpenMP 程序，科研和工程计算。
- 特点：偏向 调试和并行正确性，而不是单纯性能采样。
