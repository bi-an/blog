---
title: Coredump
date: 2024-01-23 21:42:51
categories: c/cpp
tags: debug
---

## `ulimit`

```bash
#查看配置
ulimit -a
#设置core file size
ulimit -c unlimited
```

`ulimit`只对当前终端有效。

以下两种方法对所有用户和终端有效：

1. 在`/etc/security/limits.conf`中设置（redhat衍生系linux）。
2. 或注释掉`/etc/profile`中的这一行：
    ```bash
    # No core files by default
    ulimit -S -c 0 > /dev/null 2>&1
    ```

## core_pattern

### core_pattern解释

见[链接](https://stackoverflow.com/questions/2065912/core-dumped-but-core-file-is-not-in-the-current-directory)。

Read [/usr/src/linux/Documentation/sysctl/kernel.txt](http://www.kernel.org/doc/Documentation/sysctl/kernel.txt).

    core_pattern is used to specify a core dumpfile pattern name.

在系统启动时，[Apport](https://wiki.ubuntu.com/Apport)（crash reporting service）会生成配置文件`/proc/sys/kernel/core_pattern`。参考[这里](https://askubuntu.com/questions/420410/how-to-permanently-edit-the-core-pattern-file)。

Apport uses `/proc/sys/kernel/core_pattern` to directly pipe the core dump into `apport`:

```bash
$ cat /proc/sys/kernel/core_pattern
|/usr/share/apport/apport %p %s %c
$ 
```

Note that even if `ulimit` is set to disabled core files (by specyfing a core file size of zero using `ulimit -c 0`), `apport` will still capture the crash.

For intercepting Python crashes it installs a `/etc/python*/sitecustomize.py` to call apport on unhandled exceptions.

其中，`/usr/share/apport/apport`是一个python脚本。

以下是core_pattern文件的参数说明（参考Linux Manual Page：`man core`）：
```
%c - Core file size soft resource limit of crashing process (since Linux 2.6.24).
%p - insert pid into filename 添加pid
%u - insert current uid into filename 添加当前uid
%g - insert current gid into filename 添加当前gid
%s - insert signal that caused the coredump into the filename 添加导致产生core的信号
%t - insert UNIX time that the coredump occurred into filename 添加core文件生成时的unix时间
%h - insert hostname where the coredump happened into filename 添加主机名
%e - insert coredumping executable name into filename 添加命令名

If the first character of the pattern is a '|', the kernel will treat the rest of the pattern as a command to run. The core dump will be written to the standard input of that program instead of to a file.
```

Apport的拦截组件默认是关闭的：

Apport itself is running at all times because it collects crash data for whoopsie (see [ErrorTracker](https://wiki.ubuntu.com/ErrorTracker)). However, the crash interception component is still disabled. To enable it permanently, do:

```bash
sudo nano /etc/apport/crashdb.conf
```

... and add a hash symbol # in the beginning of the following line:

    'problem_types': ['Bug', 'Package'],

To disable crash reporting just remove the hash symbol.

### 设置core_pattern

见[链接](https://www.cnblogs.com/xiaodoujiaohome/p/6222895.html)。

1. /proc/sys/kernel/core_uses_pid可以控制产生的core文件的文件名中是否添加pid作为扩展，如果添加则文件内容为1，否则为0；

2. /proc/sys/kernel/core_pattern可以设置格式化的core文件保存位置或文件名：

    ```bash
    $ cat /proc/sys/kernel/core_pattern
    |/usr/share/apport/apport %p %s %c
    $ echo "/corefile/core-%e-%p-%t" > /proc/sys/kernel/core_pattern
    ```


    你可以用下列方式来完成：
    ```bash
    #查看所有sysctl所有变量的值。
    sysctl -a
    #设置变量kernel.core_pattern为如下值。
    sudo sysctl -w kernel.core_pattern=/tmp/core-%e.%p.%h.%t
    ```

    这些操作一旦计算机重启，则会丢失，如果你想持久化这些操作，可以在 /etc/sysctl.conf文件中增加：
    ```
    kernel.core_pattern=/tmp/core%p
    ```

    加好后，如果你想不重启看看效果的话，则用下面的命令：
    ```
    sysctl -p /etc/sysctl.conf
    ```

## 相关命令行工具

参考资料：

Linux Manual Page: [`man core`](https://man7.org/linux/man-pages/man5/core.5.html)

    SEE ALSO
        bash(1),  coredumpctl(1),  gdb(1),  getrlimit(2), mmap(2), prctl(2), sigaction(2), elf(5), proc(5), pthreads(7), signal(7), systemd-coredump(8)

## Segment Fault排查

参考[链接](https://www.wtango.com/%E6%AE%B5%E9%94%99%E8%AF%AFsegmentation-fault%E4%BA%A7%E7%94%9F%E7%9A%84%E5%8E%9F%E5%9B%A0%E4%BB%A5%E5%8F%8A%E8%B0%83%E8%AF%95%E6%96%B9%E6%B3%95/)。

1. dmesg + nm + addr2line

   addr2line只能找出executable的行号；如果是shared libraries，请使用gdb。参考[这里](https://stackoverflow.com/questions/2549214/interpreting-segfault-messages)。

   dmesg输出的含义：

   ip: 表示instruction pointer.

2. fprintf
3. gdb
4. signal(SIGSEGV,handler)
5. valgrind
参考：
    [博客](https://jvns.ca/blog/2018/04/28/debugging-a-segfault-on-linux/)
    [文档](https://valgrind.org/docs/manual/ms-manual.html)
    [可以使用PostScript查看图形化结果](https://courses.cs.washington.edu/courses/cse326/05wi/valgrind-doc/ms_main.html)

1. heaptrack

    见[github仓库](https://github.com/KDE/heaptrack)

2. Jemalloc

    见[官网](https://jemalloc.net/)
