---
title: Getting Started with IB
categories: Programming
tags: IB
date: 2023-12-06 09:41:56
---

## 文档

[Documentation: RDMA Aware Networks Programming User Manual v1.6](https://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=346D319BA2A9F06B8C49B2C0AAFD28D9?doi=10.1.1.668.4459&rep=rep1&type=pdf)

[Local Documentation](/blog/documents/RDMA-Aware-Networks-Programming.pdf)

[Sun Network QDR InfiniBand Gateway Switch Topic Set](https://docs.oracle.com/cd/E19671-01/835-0793-03/z40000419112.html#scrolltoc)

[RDMA知乎专栏](https://www.zhihu.com/column/c_1231181516811390976)


用户态的Verbs API手册跟代码在一个仓库维护，手册地址：https://github.com/linux-rdma/rdma-core/tree/master/libibverbs/man

有很多在线的man page网站可以查阅这些接口的说明，比如官方的连接：https://man7.org/linux/man-pages/man3/ibv_post_send.3.html

也有一些其他非官方网页，支持在线搜索：https://linux.die.net/man/3/ibv

查阅系统man page
如果你使用的商用OS安装了rdma-core或者libibverbs库，那么可以直接用man命令查询接口：

```
man ibv_post_send
```

查询Mellanox的编程手册
《[RDMA Aware Networks Programming User Manual Rev 1.7](https://docs.nvidia.com/networking/display/rdmaawareprogrammingv17)》，最新版是2015年更新的。该手册写的比较详尽，并且附有示例程序，但是可能与最新的接口有一些差异。

Book: [Linux Kernel Networking - Implementation and Theory](https://github.com/faquir-1990/itBooks/blob/master/Linux%20Kernel%20Networking%20-%20Implementation%20and%20Theory.pdf)

[Dotan's blog](http://www.rdmamojo.com/): Dotan Barak, an InfiniBand Expert. Dotan is a Senior Software Manager at Mellanox Technologies working on RDMA Technologies.

## 性能分析工具（profiling）

Blog: [Tips and tricks to optimize your RDMA code](https://www.rdmamojo.com/2013/06/08/tips-and-tricks-to-optimize-your-rdma-code/)

[libibprof](https://github.com/mellanox-hpc/libibprof)