---
title: RDMA 建立连接
date: 2025-09-23 16:30:08
categories: RDMA
tags: InfiniBand
---

## RDMA vs TCP/IP 连接标识字段

| 字段名称 |           全称           |                                               作用说明                                               |
| :------: | :----------------------: | :--------------------------------------------------------------------------------------------------: |
|   GID    |    Global Identifier     | 标识设备，类似于 IP 地址，128 位；RoCEv2 中为 IPv6 映射地址，InfiniBand 中由设备 GUID + 路由信息生成 |
|   QPN    |    Queue Pair Number     |              标识会话，每个 RDMA 会话使用一个 QP，QPN 是其唯一编号，类似于 TCP 的端口号              |
|   PSN    |  Packet Sequence Number  |                   初始化连接时使用，用于数据包排序和连接建立，类似于 TCP 的序列号                    |
|   LID    | Local Identifier（可选） |           InfiniBand fabric 中的本地路由地址，类似于交换机端口编号，仅在 InfiniBand 中使用           |

## InfiniBand 的连接建立流程（RC 模式）

|  阶段   |     状态     |       含义       |           关键配置项           |
| :-----: | :----------: | :--------------: | :----------------------------: |
| 1️⃣ INIT | IBV_QPS_INIT |    初始化 QP     |  本地端口、访问权限、P_Key 等  |
| 2️⃣ RTR  | IBV_QPS_RTR  | Ready to Receive | 远端 QPN、LID/GID、PSN、MTU 等 |
| 3️⃣ RTS  | IBV_QPS_RTS  |  Ready to Send   | 本地 PSN、重试参数、超时设置等 |

类比 TCP 的三次握手。

为什么不像 TCP 那样自动握手？ InfiniBand 的设计目标是：

- 极低延迟：避免协议协商，直接由应用控制连接建立。
- 高吞吐：硬件直接处理数据流，减少内核干预。
- 可预测性强：状态转换明确，调试和优化更容易。
- 适用于受控环境：如 HPC、AI 训练、数据库加速，通常用于 HPC 或数据中心，不像 TCP 那样暴露在公网，不
  需要防止恶意连接。（所以 QPN 一般是用户指定，且常常是顺序的；这与 TCP 的初始序列号截然不同）

注：MTU 对比表

|              技术类型               |         支持的 MTU 值（字节）          |                          说明                           |
| :---------------------------------: | :------------------------------------: | :-----------------------------------------------------: |
|          Ethernet（标准）           |                  1500                  |          最常见的默认值，适用于大多数网络设备           |
|             InfiniBand              |       256, 512, 1024, 2048, 4096       |                固定值，由硬件和驱动支持                 |
| RoCE (RDMA over Converged Ethernet) | 通常 ≤ Ethernet MTU（如 1500 或 9000） | 实际使用值需减去 RoCE 协议头和 CRC，常见为 1024 或 4096 |
|                iWARP                |     受 TCP/IP 栈限制，通常 ≤ 1500      |              依赖传统以太网 MTU，性能受限               |

InfiniBand 的 MTU 值由头文件 `<infiniband/verbs.h>` 定义：

```c
enum ibv_mtu {
	IBV_MTU_256  = 1,
	IBV_MTU_512  = 2,
	IBV_MTU_1024 = 3,
	IBV_MTU_2048 = 4,
	IBV_MTU_4096 = 5
};
```

## 步骤

[源码](https://infiniband-doc.readthedocs.io/zh-cn/latest/8_ibv_examples/8_2_RDMA_RC_code.html)

### resources_create

1. TCP 建立连接 → 保存到 res.sock
2. ibv_get_device_list
3. ibv_get_device_name → 保存到 res.dev_name
4. ibv_open_device → 保存到 res.ib_ctx
5. ibv_query_port → 保存到 res.ib_port, res.port_attr
6. ibv_alloc_pd → 保存到 res.pd
7. ibv_create_cq → 保存到 res.cq
8. malloc → 保存到 res.buf
9. ibv_reg_mr → 保存到 res.mr
10. ibv_create_qp → 保存到 res.qp

### connect_qp

1. 如果指定 gid_idx ，则以 ib_ctx 和 ib_port 调用 ibv_query_gid 查询本地的 gid；否则用 0 初始化
   gid。
2. 填充 cm_con_data_t 结构体：`addr`(即 `res.buf`)、`mr->rkey`、`qp->qp_num`、`port_attr.lid`
3. 通过 TCP socket 交换 cm_con_data_t 的信息，将远端 cm_con_data_t 保存到 res.remote_props
4. 调用 modify_qp_to_init（使用 ibv_modify_qp 修改 QP 的状态：RESET → INIT）

|                                               参数字段                                               | 作用描述                                 | 是否必须设置 | 设置原因                                                     |
| :--------------------------------------------------------------------------------------------------: | ---------------------------------------- | ------------ | ------------------------------------------------------------ |
|                                     attr.qp_state = IBV_QPS_INIT                                     | 设置 QP 的目标状态为 INIT                | 是           | 明确告诉 HCA 要将 QP 转换到 INIT 状态                        |
|                                    attr.port_num = config.ib_port                                    | 指定使用的物理端口号                     | 是           | 多端口设备必须指定端口，否则无法建立连接                     |
|                                         attr.pkey_index = 0                                          | 设置 P_Key 索引                          | 是           | InfiniBand 使用 P_Key 进行分区管理，默认使用索引 0           |
| `attr.qp_access_flags = IBV_ACCESS_LOCAL_WRITE \| IBV_ACCESS_REMOTE_READ \| IBV_ACCESS_REMOTE_WRITE` | 设置本地和远程访问权限                   | 是           | 决定远程节点是否能读写你的内存，必须在 INIT 状态设置         |
|          `flags = IBV_QP_STATE \| IBV_QP_PKEY_INDEX \| IBV_QP_PORT \| IBV_QP_ACCESS_FLAGS`           | 指定哪些字段有效并应用到 QP 属性结构体中 | 是           | ibv_modify_qp 需要知道哪些字段是有效的，否则不会应用这些设置 |

注：InfiniBand 的分区类似以太网的 VLAN

- 同一分区内的节点可以互通
- 不同分区之间默认无法通信
- 分区成员可以通过默认分区与 Subnet Manager 通信（如 IO 节点）

5. 客户端调用 post_receive
   1. 准备 scatter/gatter entry：sge.addr, sge.length, seg.lkey
   2. 准备 receive work request: wr_id, next（多个 receive wr 可以组成一个链表，一起提交）,
      sge_list（sge 数组，多个不同内存区域的 sge 可一起提交）。
   3. 调用 ibv_post_recv 提交 receive wr。
6. modify_qp_to_rtr：修改 QP 状态为 Readay to Receive：INIT → RTR

|          字段/参数名           |    类型/值     |                                        说明                                         |
| :----------------------------: | :------------: | :---------------------------------------------------------------------------------: |
|           remote_qpn           |    uint32_t    |                            远端 QP 编号，用于建立连接。                             |
|              dlid              |    uint16_t    | Destination LID：远端设备的本地标识符，用于本地路由。d 表示 destination（目的地）。 |
|              dgid              |   uint8_t\*    | Destination GID：远端设备的全局标识符，用于全局寻址。d 表示 destination（目的地）。 |
|         attr.qp_state          |  IBV_QPS_RTR   |                          设置 QP 状态为 Ready to Receive。                          |
|         attr.path_mtu          |  IBV_MTU_256   |                          设置路径最大传输单元为 256 字节。                          |
|        attr.dest_qp_num        |   remote_qpn   |                                 指定远端 QP 编号。                                  |
|          attr.rq_psn           |       0        |                               接收队列的初始包序号。                                |
|    attr.max_dest_rd_atomic     |       1        |                     远端最多可处理的 RDMA Read/Atomic 请求数。                      |
|       attr.min_rnr_timer       |      0x12      |                    最小 RNR 重试等待时间（单位为 655.36 微秒）。                    |
|     attr.ah_attr.is_global     |     0 或 1     |                              是否启用全局路由（GID）。                              |
|       attr.ah_attr.dlid        |      dlid      |                                   设置远端 LID。                                    |
|        attr.ah_attr.sl         |       0        |                             服务等级（Service Level）。                             |
|   attr.ah_attr.src_path_bits   |       0        |                                源路径位，通常为 0。                                 |
|     attr.ah_attr.port_num      | config.ib_port |                                    本地端口号。                                     |
|     attr.ah_attr.grh.dgid      |      dgid      |                         设置远端 GID（如果启用全局路由）。                          |
|  attr.ah_attr.grh.flow_label   |       0        |                                    GRH 流标签。                                     |
|   attr.ah_attr.grh.hop_limit   |       1        |                                   GRH 跳数限制。                                    |
|  attr.ah_attr.grh.sgid_index   | config.gid_idx |                                   本地 GID 索引。                                   |
| attr.ah_attr.grh.traffic_class |       0        |                                   GRH 流量类别。                                    |
|             flags              |   多个宏组合   |                               指定哪些属性字段有效。                                |
|        ibv_modify_qp()         |    函数调用    |                               执行 QP 状态修改操作。                                |

7. modify_qp_to_rts

|          参数字段           |         含义          |                              说明                              |
| :-------------------------: | :-------------------: | :------------------------------------------------------------: |
| attr.qp_state = IBV_QPS_RTS |  设置 QP 状态为 RTS   |        表示该 QP 已准备好发送数据，是连接建立的最后一步        |
|     attr.timeout = 0x12     |       超时时间        |  单位为 4.096μs，表示发送请求等待 ACK 的最大时间。0x12 ≈ 75μs  |
|     attr.retry_cnt = 6      |       重试次数        |                 如果未收到 ACK，最多重试 6 次                  |
|     attr.rnr_retry = 0      |     RNR 重试次数      | 对方未准备好接收时的重试次数。0 表示不重试（通常用于 UC 类型） |
|       attr.sq_psn = 0       | Send Queue 的初始 PSN |   PSN（Packet Sequence Number）用于包顺序控制，需与远端匹配    |
|   attr.max_rd_atomic = 1    | 最大 RDMA Read 请求数 |          表示本端最多允许一个未完成的 RDMA Read 请求           |
|            flags            |        标志位         |  指定哪些字段在 ibv_modify_qp() 中有效，必须与设置的字段匹配   |

8. 用 TCP socket 同步交换 dummy 数据，以确保双方都进入 RTS 状态。

### post_send
