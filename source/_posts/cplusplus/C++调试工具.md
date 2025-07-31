---
title: C++ 调试工具
date: 2024-01-23 21:27:05
categories: c/cpp
tags: debug
---

## Unit testing framework

[CppTest](https://cpptest.sourceforge.io/tutorial.html)

## Memory check

valgrind
参考：
    [博客](https://jvns.ca/blog/2018/04/28/debugging-a-segfault-on-linux/)
    [文档](https://valgrind.org/docs/manual/ms-manual.html)
    [可以使用PostScript查看图形化结果](https://courses.cs.washington.edu/courses/cse326/05wi/valgrind-doc/ms_main.html)

[AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)


## debug tool

### gdb

```gdb
(gdb) breakpoint exit
(gdb) breakpoint _exit
(gdb) breakpoint atexit
(gdb) breakpoint abort
```


Enable coredump: [how to do](https://medium.com/@sourabhedake/core-dumps-how-to-enable-them-73856a437711)

```bash
ulimit -c unlimited
```

Where is the core dumped file:

```bash
grep 'kernel.core_pattern' /etc/sysctl.conf
```

### strace

Example:

```bash
strace -f -o strace.log -tt -y -yy -e trace=desc,process,network
```

Refer to [here](https://gist.github.com/graste/929bb122c353bdd90c20)

> -e trace=ipc – communication between processes (IPC)
> -e trace=memory – memory syscalls
> -e trace=network – network syscalls
> -e trace=process – process calls (like fork, exec)
> -e trace=signal – process signal handling (like HUP, exit)
> -e trace=file – file related syscalls
> -e trace=desc – all file descriptor related system calls

