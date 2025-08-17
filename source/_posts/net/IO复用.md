---
title: IO复用
date: 2025-08-16 23:40:06
categories: 网络
tags:
    - 网络
    - 文件
---

## 高效的IO函数

- sendfile
- mmap + write
- splice
- readv

## 零拷贝 (Zero copy)

- mmap + write
- sendfile
- splice

https://www.xiaolincoding.com/os/8_network_system/zero_copy.html#%E5%A6%82%E4%BD%95%E5%AE%9E%E7%8E%B0%E9%9B%B6%E6%8B%B7%E8%B4%9D

## 散布读写 (Scatter read/write)

- readv
- writev

散布读写支持一次性将数据从文件描述符读写到多个缓冲区：

- 避免多次系统调用；
- 直接分块读取，不需要额外用户态 memcpy 到不同的块。

## 参考

https://blog.csdn.net/salmonwilliam/article/details/112347938


