---
title: Getting Started with IB
categories: Network
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
《[RDMA Aware Networks Programming User Manual Rev 1.7](https://docs.nvidia.com/networking/display/rdmaawareprogrammingv17)》，最新版是2015年更新的。该手册写的比较详尽，并且附有示例程序，但是可能与最新的接口有一些差异。Mellanox VPI®（Virtual Procotol Interconnect）架构为同时支持InfiniBand和以太网语义的网络适配器和交换机提供高性能、低延迟和可靠的方法。

Book: [Linux Kernel Networking - Implementation and Theory](https://github.com/faquir-1990/itBooks/blob/master/Linux%20Kernel%20Networking%20-%20Implementation%20and%20Theory.pdf)

[Dotan's blog](http://www.rdmamojo.com/): Dotan Barak, an InfiniBand Expert. Dotan is a Senior Software Manager at Mellanox Technologies working on RDMA Technologies.

## 性能分析工具（profiling）

Blog: [Tips and tricks to optimize your RDMA code](https://www.rdmamojo.com/2013/06/08/tips-and-tricks-to-optimize-your-rdma-code/)

[libibprof](https://github.com/mellanox-hpc/libibprof)

## IB简介

RDMA - Remote Direct Memory Access 远程直接内存存取。

InfiniBand是一种高性能计算机网络通信标准，它具有极高的吞吐量和极低的延迟。如果您需要使用InfiniBand进行编程，您需要使用支持InfiniBand的编程语言（如C++）来编写代码。

机构和组织：

OFA: [Open Fabrics Alliance](https://www.openfabrics.org/).

IBTA: [InfiniBand Trade Association](https://www.infinibandta.org/).

## 概念

* `CQ` - Complete Queue 完成队列
* `WQ` - Work Queue 工作队列
* `WR` - Work Request 工作请求
* `QP` - Queue Pairs 队列对（Send-Receive）
* `SQ` - Send Queue 发送队列
* `RQ` - Receive Queue 接收队列
* `PD` - Protection Domain 保护域，将QP和MR结合在一起
* `MR` - Memory Region 内存区域。一块经注册过的且本地网卡可以读写的内存区域。包含R_Key和L_Key。
* `SGE` - Scatter/Gather Elements 分散/聚集元素。
* `R_Key` - Remote Key
* `L_Key` - Local Key
* `CA` - (Host) Channel Adapter, an inifiniband network interface card.
* `NIC` - Network Interface Card 网卡。
* `LID` - Local Identifier.
* `CM` - Connection Manager.

其他常见缩写：

* `RC` - reliable connected.
* `SCSI` - Small Computer System Interface 小型计算机系统接口。
* `SRP` - SCSI RDMA Protocol. / Secure Remote Password.

博客：https://blog.51cto.com/liangchaoxi/4044818


## 安装

[InfiniBand 和 RDMA 相关软件包](https://access.redhat.com/documentation/zh-cn/red_hat_enterprise_linux/7/html/networking_guide/sec-infiniband_and_rdma_related_software_packages)

    sudo apt-get install infiniband-diags
    sudo apt install ibverbs-utils

## API

- [introduction to programming infiniband](https://insujang.github.io/2020-02-09/introduction-to-programming-infiniband/)
- [RDMA Aware Programming user manual (PDF)](https://indico.cern.ch/event/218156/attachments/351725/490089/RDMA_Aware_Programming_user_manual.pdf)

以下是一些支持InfiniBand的C++库：

Infinity：这是一个轻量级的C++ RDMA库，用于InfiniBand网络。它提供了对两侧（发送/接收）和单侧（读/写/原子）操作的支持，并且是一个简单而强大的面向对象的ibVerbs抽象。该库使用户能够构建使用RDMA的复杂应用程序，而不会影响性能[1](https://github.com/claudebarthels/infinity)。

OFED：这是一个开放式Fabrics Enterprise Distribution，它提供了对InfiniBand和RoCE（RDMA over Converged Ethernet）技术的支持。OFED提供了一组用户空间库和驱动程序，可用于构建支持RDMA的应用程序[2](https://zhuanlan.zhihu.com/p/337461037)。

以下是使用Infinity库编写支持InfiniBand的C++代码示例：

```cpp
// 创建新上下文
infinity::core::Context *context = new infinity::core::Context();

// 创建队列对
infinity::queues::QueuePairFactory *qpFactory = new infinity::queues::QueuePairFactory(context);
infinity::queues::QueuePair *qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);

// 创建并向网络注册缓冲区
infinity::memory::Buffer *localBuffer = new infinity::memory::Buffer(context, BUFFER_SIZE);

// 从远程缓冲区读取（单向）并等待完成
infinity::memory::RegionToken *remoteBufferToken = new infinity::memory::RegionToken(REMOTE_BUFFER_INFO);
infinity::requests::RequestToken requestToken(context);
qp->read(localBuffer, remoteBufferToken, &requestToken);
requestToken.waitUntilCompleted();

// 将本地缓冲区的内容写入远程缓冲区（单向）并等待完成
qp->write(localBuffer, remoteBufferToken, &requestToken);
requestToken.waitUntilCompleted();

// 将本地缓冲区的内容通过队列对发送（双向）并等待完成
qp->send(localBuffer, &requestToken);
requestToken.waitUntilCompleted();

// 关闭连接
delete remoteBufferToken;
delete localBuffer;
delete qp;
delete qpFactory;
delete context;
```

以下是使用OFED库编写支持InfiniBand的C++代码示例：

```cpp
// 创建新上下文
struct ibv_context *context = ibv_open_device(*device);

// 创建完成端口
struct ibv_pd *pd = ibv_alloc_pd(context);

// 创建队列对
struct ibv_qp_init_attr qp_init_attr;
memset(&qp_init_attr, 0, sizeof(qp_init_attr));
qp_init_attr.send_cq = cq;
qp_init_attr.recv_cq = cq;
qp_init_attr.qp_type = IBV_QPT_RC;
qp_init_attr.cap.max_send_wr = 1;
qp_init_attr.cap.max_recv_wr = 1;
qp_init_attr.cap.max_send_sge = 1;
qp_init_attr.cap.max_recv_sge = 1;
struct ibv_qp *qp = ibv_create_qp(pd, &qp_init_attr);

// 创建并向网络注册缓冲区
char *localBuffer = (char *)malloc(BUFFER_SIZE);
struct ibv_mr *mr = ibv_reg_mr(pd, localBuffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);

// 连接到远程主机
struct sockaddr_in remoteAddress;
memset(&remoteAddress, 0, sizeof(remoteAddress));
remoteAddress.sin_family = AF_INET;
remoteAddress.sin_port = htons(PORT_NUMBER);
inet_pton(AF_INET, SERVER_IP, &remoteAddress.sin_addr);
struct rdma_cm_id *cmId;
rdma_create_id(*eventChannel, &cmId, NULL, RDMA_PS_TCP);
rdma_resolve_addr(cmId, NULL, (struct sockaddr *)&remoteAddress, RESOLVE_TIMEOUT_MS);

// 等待连接完成
rdma_wait_event(*eventChannel, RDMA_CM_EVENT_ESTABLISHED);
rdma_ack_cm_event(cmEvent);

// 获取远程缓冲区信息
struct ibv_wc wc;
ibv_post_recv(qp, &recvWr, &badRecvWr);
do {
    ibv_poll_cq(cq, 1, &wc);
} while (wc.status != IBV_WC_SUCCESS || wc.opcode != IBV_WC_RECV_RDMA_WITH_IMM || wc.imm_data != htonl(IMM_DATA));
remoteBufferInfo.rkey = ntohl(wc.imm_data >> 8);
remoteBufferInfo.vaddr = wc.wr_id;

// 将本地缓冲区的内容写入远程缓冲区（单向）
struct ibv_send_wr sendWr;
memset(&sendWr, 0, sizeof(sendWr));
sendWr.wr_id = 0;
sendWr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
sendWr.sg_list = &localSge;
sendWr.num_sge = 1;
sendWr.send_flags = IBV_SEND_SIGNALED;
sendWr.wr.rdma.remote_addr = remoteBufferInfo.vaddr;
sendWr.wr.rdma.rkey = remoteBufferInfo.rkey;
localSge.addr = (uintptr_t)localBuffer;
localSge.length = BUFFER_SIZE;
localSge.lkey = mr->lkey;
ibv_post_send(qp, &sendWr, &badSendWr);

// 关闭连接
ibv_dereg_mr(mr);
free(localBuffer);
ibv_destroy_qp(qp);
ibv_dealloc_pd(pd);
ibv_close_device(context);
```

* 入门级文档：https://zhuanlan.zhihu.com/p/337461037
* 文档：https://docs.kernel.org/infiniband/index.html
* 文档：http://blog.foool.net/wp-content/uploads/linuxdocs/infiniband.pdf
* 文档：https://support.bull.com/documentation/byproduct/infra/sw-extremcomp/sw-extremcomp-com/g
* 文档：https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html-single/configuring_infiniband_and_rdma_networks/index
* 博客：https://zhuanlan.zhihu.com/p/337461037
* 文档：https://access.redhat.com/documentation/zh-cn/red_hat_enterprise_linux/7/html/networking_guide/ch-configure_infiniband_and_rdma_networks
* 知乎专栏：https://www.zhihu.com/column/c_1231181516811390976
* 知乎专栏：https://www.zhihu.com/column/rdmatechnology

[Linux manual page](https://man7.org/linux/man-pages/man3/ibv_reg_mr.3.html)


## Command-Line


文档：https://docs.nvidia.com/networking/pages/viewpage.action?pageId=43719572

* `ibstat`
* `ibhosts` - 查看所有的IB hosts。
* `ibnetdiscover` - discover InfiniBand topology.
* `ibv_devices` - list RDMA devices.
* `ibv_devinof` - Print information about RDMA devices available for use from userspace.
* `ibv_rc_pingpong` - Run a simple ping-pong test over InfiniBand via the reliable connected (RC) transport.
* `targetcli` - administration shell for storage targets

    `targetcli` is  a  shell for viewing, editing, and saving the configuration of the kernel's target subsystem,
    also known as LIO. It enables the administrator to assign local storage resources backed by either files,
    volumes, local SCSI devices, or ramdisk, and export them to remote systems via network fabrics, such as iSCSI or FCoE.

* `srp_daemon` - Discovers and connects to InfiniBand SCSI RDMA Protocol (SRP) targets in an IB fabric.
* `ibsrpdm` - List InfiniBand SCSI RDMA Protocol (SRP) targets on an IB fabric.

## liraries

devid: device ID library. Refer to [here](https://docs.oracle.com/cd/E86824_01/html/E54772/libdevid-3lib.html).

ibverbs: 使得用户空间进程能够使用RDMA verbs（即进行RDMA操作）。Refer to [here](https://www.ibm.com/docs/en/aix/7.2?topic=ofed-libibverbs-library).

dl: Dynamic Loader.