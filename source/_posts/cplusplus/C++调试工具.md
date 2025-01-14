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

## Performance analyzer

Oracle Developer Studio：

[Performance Analyzer](https://www.oracle.com/application-development/technologies/developerstudio-features.html#performance-analyzer-tab)：[手册](https://docs.oracle.com/cd/E77782_01/html/E77798/afagg.html#OSSPAgrkam)

[Thread Analyzer](https://www.oracle.com/application-development/technologies/developerstudio-features.html#thread-analyzer-tab)

[gprof](https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_mono/gprof.html): [使用方法](https://blog.csdn.net/luronggui/article/details/118141262)


### timer

``` bash
$ /usr/bin/time -p ls
```

Or,

``` bash
$ time ls
```

其中（参考[链接](https://ostechnix.com/how-to-find-the-execution-time-of-a-command-or-process-in-linux/)），

``` bash
$ type -a time
time is a shell keyword
time is /usr/bin/time
```

### CPU时间

Function's CPU time:

    * Inclusive time: total cpu time, include all functions it calls.
    * Exclusive time: only the time used by the function itself, exclusive all its children.

    Refer to [here](https://stackoverflow.com/questions/15760447/what-is-the-meaning-of-incl-cpu-time-excl-cpu-time-incl-real-cpu-time-excl-re/74426370).

1. Wall time: total time the process used, containing IO time.
2. CPU usage (CPU利用率) = CPU time / Wall time.
3. real/user/system time
   * **Real** is wall clock time - time from start to finish of the call. This is all elapsed time including time slices used by other processes and time the process spends blocked (for example if it is waiting for I/O to complete).
   * **User** is the amount of CPU time spent in user-mode code (outside the kernel) within the process. This is only actual CPU time used in executing the process. Other processes and time the process spends blocked do not count towards this figure.
   * **Sys** is the amount of CPU time spent in the kernel within the process. This means executing CPU time spent in system calls within the kernel, as opposed to library code, which is still running in user-space. Like 'user', this is only CPU time used by the process. See below for a brief description of kernel mode (also known as 'supervisor' mode) and the system call mechanism.

    Refer to [here](https://stackoverflow.com/questions/556405/what-do-real-user-and-sys-mean-in-the-output-of-time1).

4. CPU 时间可能大于墙上时间：

   这是因为 CPU 时间是所有 CPU 核的运行时间的累加和，墙上时间则是实际的时间。此时 CPU 利用率大于 100%. （这是自己的理解）

5. TODO: Is CPU time in flame graph sum of all the CPU time? Or is it the wall time when CPU works?

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

### DDT

[DDT](https://www.linaroforge.com/linaroDdt)
