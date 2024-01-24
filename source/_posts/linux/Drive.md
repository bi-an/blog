---
title: Drive
date: 2024-01-23 21:56:44
categories: c/cpp
tags: Linux
---

# 驱动

本文介绍驱动。

## 命令

* `lsmod` - Show status of modules in the Linux kernel.

  `lsmod` is a trivial program which nicely formats the contents of the `/proc/odules`,
  showing what kernel modules are currently loaded.

* `dmesg` - examine or control the kernel ring buffer.

    The kernel buffer is a data structure used for keeping the log messages of the kernel and the kernel modules.
    It's a ring buffer with a fixed size. Once it's full, new messages overwrite the oldest messages.
    During boot, the kernel saves the messages into the kernel buffer.

* `lsscsi` - Uses information in sysfs (Linux kernel series 2.6 and later) to list SCSI devices (or hosts) currently attached to the system.