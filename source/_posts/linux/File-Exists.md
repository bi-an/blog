---
title: File Exists
date: 2024-09-06 18:09:03
categories: Linux
tags: c/cpp
---

# 判断文件是否存在

## 相关函数

- `access`
- `stat`
- `inotify_node`
- `opendir`
- `readdir`

## 注意事项

文件系统并不会实时刷新缓存，尤其是在网络文件系统中。这会导致文件即使已经创建，`access`或`stat`函数依然返回"No such file"。
但是`ls`可以看到文件，这是因为`ls`和`access`函数的实现机制不同。

我们可以改用`readdir`来读取目录。

