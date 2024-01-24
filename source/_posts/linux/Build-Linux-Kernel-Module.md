---
title: Build Linux Kernel Module
date: 2024-01-23 22:24:57
categories: c/cpp
tags: Linux
---

## 内核编译

obj-m: [链接](https://stackoverflow.com/questions/57839941/what-is-the-meaning-of-obj-m-in-linux-device-driver-makefile)

make modules: [链接](https://askubuntu.com/questions/1363530/what-does-make-and-make-modules-do-when-compiling-building-the-kernel)

## 编译内核模块

[文档](https://tldp.org/LDP/lkmpg/2.6/html/x181.html)

Makefile:

    obj-m += hello-1.o

    all:
    	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

    clean:
    	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

`make -C` changes to a new directory where the new makefiles will be run.

`M=$(PWD)` defines a variable `M` which the child makefiles can use.

## 命令

- `insmod`: 插入一个模块（.ko）到kernel中。
- `rmmod`: 从内核中删除一个模块。
- `modprobe`: 从内核中增删模块。`modprobe`从`` /lib/modules/`uname -r` ``中搜索所有的模块和其他文件，除了`/etc/modprobe.d`目录中的可选配置文件（参见`modprobe.d(5)`）。
- `lsmod`: 显示当前加载的内核模块的状态（即对`/proc/modules`内容的格式化）。
- `modinfo`: 从命令行指定的内核模块中提取信息，如果没有给出模块名称，那么会搜索`/lib/modules/version`目录，就像`modprobe`加载模块时一样。
- `depmod`: 生成`modules.dep`和映射文件。
- `dmesg`: 打印或控制内核ring buffer（存储kernel message）。