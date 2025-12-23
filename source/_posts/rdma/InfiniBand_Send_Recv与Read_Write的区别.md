---
title: InfiniBand Send/Recv与Read/Write的区别
date: 2025-12-23 16:32:46
categories: RDMA
tags: InfiniBand RDMA
---

## 引言

InfiniBand/RDMA 提供了两种截然不同的数据传输模式：**Send/Recv** 和 **Read/Write**。这两种模式在底层实现机制、CPU 参与度、对端感知性和传输控制策略方面存在根本性差异。

## 核心差异对比

### 基本特性对比

| 维度 | Send/Recv | Read/Write |
|:---:|:---------:|:----------:|
| **操作码** | `IBV_WR_SEND` | `IBV_WR_RDMA_WRITE`<br>`IBV_WR_RDMA_READ` |
| **CPU 参与** | 接收端必须参与（post recv） | 对端 CPU 不参与 |
| **对端感知** | 完全感知（CQ 事件） | 不感知（无事件） |
| **流控机制** | 有（RNR 机制） | 无 |
| **传输速率控制** | 受接收端节制 | 不受对端节制 |
| **内存注册要求** | 发送端和接收端都需要 | 发起端和对端都需要（对端需设置 REMOTE 权限） |
| **远程地址信息** | 不需要 | 需要（remote_addr + rkey） |
| **完成事件** | 双方都有 | 仅发起端有 |
| **使用复杂度** | 需要双方协调 | 只需发起端操作 |
| **性能** | 高（硬件加速） | 最高（零拷贝，无 CPU 参与） |

### 工作机制对比

| 步骤 | Send/Recv | Read/Write |
|:---:|:---------:|:----------:|
| **1. 准备工作** | 接收端预先 post recv | 对端注册内存并传递 R_Key |
| **2. 发起操作** | 发送端 post send | 发起端 post RDMA Read/Write |
| **3. 数据传输** | HCA 匹配 Receive WR 并写入缓冲区 | HCA 直接访问远程内存 |
| **4. 完成通知** | 双方 CQ 都有完成事件 | 仅发起端 CQ 有完成事件 |
| **5. 对端状态** | 接收端知道数据到达 | 对端 CPU 完全无感知 |

### CPU 参与度与对端感知性

| 方面 | Send/Recv | Read/Write |
|:---:|:---------:|:----------:|
| **接收端操作** | 必须预先 post recv | 无需任何操作 |
| **接收端 CPU** | 必须参与，处理 CQ 事件 | 完全不参与 |
| **接收端控制** | 可以控制接收速率 | 无法控制传输速率 |
| **对端 CPU 状态** | 需要处理接收事件 | 可以执行其他任务或休眠 |
| **数据到达感知** | 完全感知（CQ 事件） | 不感知 |
| **传输时机控制** | 可以控制接收时机 | 无法控制传输时机 |
| **数据来源信息** | 知道数据来源（QP 编号） | 不知道数据来源 |
| **数据大小信息** | 知道数据大小 | 不知道传输大小 |
| **传输完成感知** | 通过 CQ 事件知道 | 无法直接知道 |
| **同步机制** | 通过 CQ 事件同步 | 需要额外机制（Send/Recv 或轮询） |

### 传输速率控制

| 方面 | Send/Recv | Read/Write |
|:---:|:---------:|:----------:|
| **流控机制** | RNR（Receiver Not Ready）机制 | 无流控机制 |
| **速率控制方** | 接收端控制（通过 post recv 频率） | 发起端完全控制 |
| **RNR 处理** | 发送端收到 RNR NACK，等待重试 | 不适用 |
| **对端限制能力** | 可以限制接收速率 | 无法限制传输速率 |
| **潜在风险** | 接收端可能过载（但可通过流控避免） | 对端可能被数据淹没 |
| **性能特点** | 受接收端处理能力限制 | 可达到网络带宽上限 |

**RNR 机制参数**：

| 参数 | 说明 | 典型值 |
|:---:|:---|:---|
| `rnr_retry` | RNR 重试次数（0-7，7 表示无限重试） | 7 |
| `min_rnr_timer` | 最小 RNR 等待时间（单位：655.36 微秒） | 0x12 |

```c
// 在 modify_qp_to_rts 中设置
attr.rnr_retry = 7;          // RNR 重试次数
attr.min_rnr_timer = 0x12;   // 最小 RNR 等待时间
```

## API 与代码示例

### 关键函数

#### ibv_post_recv()

接收端必须预先调用此函数提交 Receive WR：

```c
#include <infiniband/verbs.h>

int ibv_post_recv(struct ibv_qp *qp, 
                  struct ibv_recv_wr *wr,
                  struct ibv_recv_wr **bad_wr);
```

| 参数 | 说明 |
|:---:|:---|
| `qp` | 队列对句柄 |
| `wr` | Receive Work Request 链表头 |
| `bad_wr` | 如果提交失败，返回失败的 WR 指针 |

**返回值**：成功返回 0，失败返回错误码

#### ibv_post_send()

发送端调用此函数提交 Send WR 或 RDMA Read/Write WR：

```c
int ibv_post_send(struct ibv_qp *qp,
                  struct ibv_send_wr *wr,
                  struct ibv_send_wr **bad_wr);
```

| 参数 | 说明 |
|:---:|:---|
| `qp` | 队列对句柄 |
| `wr` | Send Work Request 链表头 |
| `bad_wr` | 如果提交失败，返回失败的 WR 指针 |

### 数据结构

#### Receive Work Request

```c
struct ibv_recv_wr {
    uint64_t                wr_id;      // 工作请求标识符
    struct ibv_recv_wr     *next;       // 下一个 WR（可组成链表）
    struct ibv_sge         *sg_list;    // Scatter/Gather 元素数组
    int                     num_sge;    // SGE 数量
};
```

#### Send Work Request

```c
struct ibv_send_wr {
    uint64_t                wr_id;           // 工作请求标识符
    struct ibv_send_wr     *next;            // 下一个 WR
    struct ibv_sge         *sg_list;         // Scatter/Gather 元素数组
    int                     num_sge;         // SGE 数量
    enum ibv_wr_opcode      opcode;          // 操作类型
    int                     send_flags;      // 发送标志
    union {
        struct {
            uint64_t        remote_addr;     // 远程地址（RDMA 操作使用）
            uint32_t        rkey;            // 远程密钥（RDMA 操作使用）
        } rdma;
        // ... 其他操作类型
    } wr;
    uint32_t                imm_data;        // 立即数据（可选）
};
```

#### Scatter/Gather Element

```c
struct ibv_sge {
    uint64_t        addr;    // 缓冲区地址
    uint32_t        length;  // 缓冲区长度
    uint32_t        lkey;    // 本地内存区域密钥
};
```

### 操作码与标志位

| 操作码 | 说明 | 使用场景 |
|:---:|:---|:---|
| `IBV_WR_SEND` | Send/Recv 模式 | 需要接收端确认的消息传递 |
| `IBV_WR_RDMA_WRITE` | RDMA Write | 将本地数据写入远程内存 |
| `IBV_WR_RDMA_WRITE_WITH_IMM` | RDMA Write with Immediate | 写入数据并通知对端 |
| `IBV_WR_RDMA_READ` | RDMA Read | 从远程内存读取数据 |

| 标志位 | 说明 | 使用场景 |
|:---:|:---|:---|
| `IBV_SEND_FENCE` | 栅栏标志，确保顺序 | 需要保证操作顺序 |
| `IBV_SEND_SIGNALED` | 请求完成通知 | 需要知道操作完成 |
| `IBV_SEND_SOLICITED` | 请求立即通知（用于 Send with Immediate） | Send with Immediate |
| `IBV_SEND_INLINE` | 内联发送（小数据直接放在 WR 中） | 小数据快速发送 |

### Send/Recv 代码示例

#### 接收端代码

```c
// 1. 准备接收缓冲区
char *recv_buffer = malloc(BUFFER_SIZE);
struct ibv_mr *recv_mr = ibv_reg_mr(pd, recv_buffer, BUFFER_SIZE,
                                    IBV_ACCESS_LOCAL_WRITE);

// 2. 准备 Scatter/Gather Element
struct ibv_sge recv_sge;
recv_sge.addr = (uintptr_t)recv_buffer;
recv_sge.length = BUFFER_SIZE;
recv_sge.lkey = recv_mr->lkey;

// 3. 准备 Receive Work Request
struct ibv_recv_wr recv_wr, *bad_recv_wr;
memset(&recv_wr, 0, sizeof(recv_wr));
recv_wr.wr_id = (uintptr_t)recv_buffer;
recv_wr.sg_list = &recv_sge;
recv_wr.num_sge = 1;

// 4. 提交 Receive WR（必须预先提交）
if (ibv_post_recv(qp, &recv_wr, &bad_recv_wr)) {
    fprintf(stderr, "Failed to post receive WR\n");
    return -1;
}

// 5. 等待接收完成
struct ibv_wc wc;
int ne;
do {
    ne = ibv_poll_cq(cq, 1, &wc);
} while (ne == 0);

if (wc.status != IBV_WC_SUCCESS) {
    fprintf(stderr, "Receive failed with status: %s\n",
            ibv_wc_status_str(wc.status));
    return -1;
}
```

#### 发送端代码

```c
// 1. 准备发送缓冲区
char *send_buffer = malloc(BUFFER_SIZE);
memcpy(send_buffer, data_to_send, data_size);
struct ibv_mr *send_mr = ibv_reg_mr(pd, send_buffer, BUFFER_SIZE,
                                     IBV_ACCESS_LOCAL_WRITE);

// 2. 准备 Scatter/Gather Element
struct ibv_sge send_sge;
send_sge.addr = (uintptr_t)send_buffer;
send_sge.length = data_size;
send_sge.lkey = send_mr->lkey;

// 3. 准备 Send Work Request
struct ibv_send_wr send_wr, *bad_send_wr;
memset(&send_wr, 0, sizeof(send_wr));
send_wr.wr_id = (uintptr_t)send_buffer;
send_wr.opcode = IBV_WR_SEND;              // Send/Recv 模式
send_wr.send_flags = IBV_SEND_SIGNALED;    // 请求完成通知
send_wr.sg_list = &send_sge;
send_wr.num_sge = 1;

// 4. 提交 Send WR
if (ibv_post_send(qp, &send_wr, &bad_send_wr)) {
    fprintf(stderr, "Failed to post send WR\n");
    return -1;
}

// 5. 等待发送完成
struct ibv_wc wc;
int ne;
do {
    ne = ibv_poll_cq(cq, 1, &wc);
} while (ne == 0);
```

### Read/Write 代码示例

#### RDMA Write 操作

```c
// 发起端代码
struct ibv_send_wr write_wr, *bad_wr;
memset(&write_wr, 0, sizeof(write_wr));

write_wr.wr_id = (uintptr_t)local_buffer;
write_wr.opcode = IBV_WR_RDMA_WRITE;        // RDMA Write
write_wr.send_flags = IBV_SEND_SIGNALED;
write_wr.sg_list = &sge;
write_wr.num_sge = 1;
write_wr.wr.rdma.remote_addr = remote_addr;  // 远程内存地址
write_wr.wr.rdma.rkey = remote_rkey;         // 远程内存密钥

ibv_post_send(qp, &write_wr, &bad_wr);
```

#### RDMA Read 操作

```c
struct ibv_send_wr read_wr, *bad_wr;
memset(&read_wr, 0, sizeof(read_wr));

read_wr.wr_id = (uintptr_t)local_buffer;
read_wr.opcode = IBV_WR_RDMA_READ;          // RDMA Read
read_wr.send_flags = IBV_SEND_SIGNALED;
read_wr.sg_list = &sge;
read_wr.num_sge = 1;
read_wr.wr.rdma.remote_addr = remote_addr;  // 远程内存地址
read_wr.wr.rdma.rkey = remote_rkey;         // 远程内存密钥

ibv_post_send(qp, &read_wr, &bad_wr);
```

#### 内存注册要求

```c
// 对端注册内存（允许远程访问）
struct ibv_mr *remote_mr = ibv_reg_mr(pd, remote_buffer, BUFFER_SIZE,
                                       IBV_ACCESS_LOCAL_WRITE |      // 本地写权限
                                       IBV_ACCESS_REMOTE_READ |      // 允许远程读
                                       IBV_ACCESS_REMOTE_WRITE);     // 允许远程写

// 将 R_Key 和地址传递给发起端
// remote_addr = (uint64_t)remote_buffer;
// remote_rkey = remote_mr->rkey;
```

## 同步机制

### Send/Recv 同步机制

**关键问题**：ibv_post_recv 必须在 ibv_post_send 之前吗？

**答案**：不是必须在 `ibv_post_send` 之前调用，但必须在数据到达之前 post recv。

#### 调用顺序说明

| 场景 | 说明 | 是否可行 |
|:---:|:---|:---|
| **预先 post recv** | 在发送端 post send 之前，接收端预先 post recv | ✅ 推荐做法 |
| **同时调用** | 接收端和发送端同时调用（不同线程/进程） | ✅ 可行，但需要确保 recv 先完成 |
| **延迟 post recv** | 发送端先 post send，接收端后 post recv | ⚠️ 可能导致 RNR 错误 |

#### 工作原理

1. **接收队列（RQ）机制**：
   - 接收端调用 `ibv_post_recv()` 将 Receive WR 提交到接收队列（RQ）
   - 当数据包到达时，HCA 硬件从 RQ 中取出一个 Receive WR
   - 如果 RQ 中没有 Receive WR，HCA 返回 RNR（Receiver Not Ready）NACK

2. **时序要求**：
   - **不是**要求 `ibv_post_recv()` 必须在 `ibv_post_send()` 之前调用
   - **而是**要求数据包到达时，RQ 中必须有可用的 Receive WR
   - 由于网络延迟，通常可以预先 post recv

#### 代码示例

**场景 1：预先批量 post recv（推荐）**

```c
// 接收端：预先批量提交多个 Receive WR
for (int i = 0; i < BATCH_SIZE; i++) {
    struct ibv_recv_wr recv_wr, *bad_wr;
    // ... 准备 recv_wr
    ibv_post_recv(qp, &recv_wr, &bad_wr);
}

// 此时发送端可以随时 post send
// 发送端：随时发送数据
ibv_post_send(qp, &send_wr, &bad_send_wr);
```

**场景 2：RNR 错误处理**

```c
// 发送端处理 RNR 错误
struct ibv_wc wc;
ibv_poll_cq(cq, 1, &wc);

if (wc.status == IBV_WC_RNR_RETRY_EXC_ERR) {
    // RNR 重试次数超限
    fprintf(stderr, "RNR retry exceeded\n");
} else if (wc.status == IBV_WC_REM_OP_ERR) {
    // 可能是 RNR 错误
    fprintf(stderr, "Remote operation error, possibly RNR\n");
}
```

#### 最佳实践

| 实践 | 说明 | 原因 |
|:---:|:---|:---|
| **预先批量 post recv** | 在连接建立后立即批量提交多个 Receive WR | 避免 RNR 错误，提高性能 |
| **保持 RQ 中有足够的 WR** | 接收完成后立即重新 post recv | 确保持续接收能力 |
| **使用应用层同步** | 如果必须延迟 post recv，使用同步机制 | 避免 RNR 错误 |
| **合理设置 RNR 参数** | 设置合适的 `rnr_retry` 和 `min_rnr_timer` | 给接收端时间 post recv |

### Read/Write 同步机制

由于 Read/Write 操作对端无感知，需要额外的同步机制来保证数据一致性和操作顺序。

#### 同步问题

| 问题 | 说明 | 影响 |
|:---:|:---|:---|
| **数据写入完成通知** | 对端不知道数据何时写入完成 | 可能读取到未完成的数据 |
| **数据读取时机** | 对端不知道何时被读取 | 可能在修改时被读取，导致数据不一致 |
| **操作顺序保证** | 多个 RDMA 操作的顺序 | 可能乱序执行，导致逻辑错误 |
| **并发访问** | 本地 CPU 和远程 RDMA 同时访问 | 可能导致数据竞争 |

#### 同步机制对比

| 机制 | 说明 | 适用场景 |
|:---:|:---|:---|
| **RDMA Write with Immediate** | 写入数据的同时发送立即数据通知 | 需要通知对端数据已写入 |
| **Send/Recv 通知** | 通过 Send/Recv 发送完成通知 | 需要确认和流控 |
| **内存屏障（Fence）** | 使用 `IBV_SEND_FENCE` 保证顺序 | 需要保证操作顺序 |
| **原子操作** | 使用原子操作作为同步点 | 需要细粒度同步 |
| **版本号/双缓冲** | 使用版本号或双缓冲机制 | 需要检测数据变化或无锁读取 |

#### RDMA Write with Immediate

在写入数据的同时，发送立即数据通知对端：

```c
// 发起端：写入数据并通知
struct ibv_send_wr write_wr, *bad_wr;
memset(&write_wr, 0, sizeof(write_wr));

write_wr.wr_id = (uintptr_t)local_buffer;
write_wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;  // 带立即数据的 Write
write_wr.send_flags = IBV_SEND_SIGNALED;
write_wr.sg_list = &sge;
write_wr.num_sge = 1;
write_wr.wr.rdma.remote_addr = remote_addr;
write_wr.wr.rdma.rkey = remote_rkey;
write_wr.imm_data = htonl(NOTIFICATION_FLAG);  // 立即数据（网络字节序）

ibv_post_send(qp, &write_wr, &bad_wr);

// 对端：接收立即数据通知（需要预先 post recv）
struct ibv_recv_wr recv_wr, *bad_recv_wr;
// ... 准备 Receive WR
ibv_post_recv(qp, &recv_wr, &bad_recv_wr);

// 轮询 CQ，接收立即数据通知
struct ibv_wc wc;
ibv_poll_cq(cq, 1, &wc);
if (wc.opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
    uint32_t imm_data = ntohl(wc.imm_data);
    // 知道数据已经写入，可以安全读取
}
```

#### 内存屏障（Fence）机制

使用 `IBV_SEND_FENCE` 保证操作顺序：

```c
// 场景：需要保证多个 RDMA Write 的顺序
struct ibv_send_wr wr[3], *bad_wr;

// WR 1: 写入元数据
wr[0].opcode = IBV_WR_RDMA_WRITE;
wr[0].send_flags = 0;
wr[0].next = &wr[1];

// WR 2: Fence，确保前面的操作完成
wr[1].opcode = IBV_WR_RDMA_WRITE;
wr[1].send_flags = IBV_SEND_FENCE;  // 栅栏标志
wr[1].next = &wr[2];

// WR 3: 写入标志位（表示数据已准备好）
wr[2].opcode = IBV_WR_RDMA_WRITE;
wr[2].send_flags = IBV_SEND_SIGNALED;
wr[2].next = NULL;

ibv_post_send(qp, &wr[0], &bad_wr);
// 保证：元数据写入 → Fence → 标志位写入（顺序执行）
```

**Fence 的作用**：
- 确保 Fence 之前的所有操作在 Fence 之后的操作之前完成
- 保证操作的全局顺序（跨多个 QP）
- 适用于需要严格顺序的场景

#### 原子操作同步

使用原子操作作为同步点：

```c
// 对端：准备数据后，使用原子操作设置标志
struct ibv_send_wr atomic_wr, *bad_wr;
memset(&atomic_wr, 0, sizeof(atomic_wr));

// 关键操作码：Compare and Swap 原子操作
// 作用：原子地比较远程内存的值，如果等于期望值则交换为新值
// 特点：这是硬件保证的原子操作，不会被其他操作打断，用于实现同步原语
atomic_wr.opcode = IBV_WR_ATOMIC_CMP_AND_SWP;

// 关键标志位：请求完成通知
// 作用：操作完成后在 CQ 中生成完成事件，用于确认原子操作是否成功
// 注意：原子操作的成功/失败通过 Work Completion 的状态字段判断
atomic_wr.send_flags = IBV_SEND_SIGNALED;
atomic_wr.sg_list = &sge;  // 本地缓冲区，用于存储旧值
atomic_wr.num_sge = 1;
atomic_wr.wr.atomic.remote_addr = remote_flag_addr;  // 远程标志位地址
atomic_wr.wr.atomic.rkey = remote_rkey;
atomic_wr.wr.atomic.compare_add = 0;      // 期望值：0（未准备好）
atomic_wr.wr.atomic.swap = 1;              // 新值：1（已准备好）

ibv_post_send(qp, &atomic_wr, &bad_wr);
```

| 操作码 | 说明 | 用途 |
|:---:|:---|:---|
| `IBV_WR_ATOMIC_CMP_AND_SWP` | Compare and Swap | 条件更新标志位 |
| `IBV_WR_ATOMIC_FETCH_AND_ADD` | Fetch and Add | 计数器操作 |

#### Read 时的并发安全

**问题**：如果 Read 时对端 CPU 正在修改数据，可能导致读取到不一致的数据。

**解决方案**：

**版本号机制**：

```c
// 数据结构
struct data_with_version {
    uint64_t version;      // 版本号
    char data[BUFFER_SIZE]; // 实际数据
};

// 对端：修改数据时增加版本号
void update_data(struct data_with_version *buf) {
    prepare_new_data(buf->data);
    __sync_synchronize();  // 内存屏障
    __sync_add_and_fetch(&buf->version, 1);  // 原子增加版本号
}

// 发起端：读取时检查版本号
uint64_t old_version = 0;
do {
    uint64_t version_before = read_version();
    __sync_synchronize();
    read_data(buffer);
    __sync_synchronize();
    uint64_t version_after = read_version();
    
    // 如果版本号相同，说明读取期间数据未变化
    if (version_before == version_after && version_before != old_version) {
        break;  // 数据一致，可以使用
    }
    old_version = version_before;
} while (1);
```

**双缓冲机制**：

```c
// 对端：使用两个缓冲区交替
struct double_buffer {
    char buffer[2][BUFFER_SIZE];
    volatile int active_index;  // 当前活跃缓冲区索引
};

// 对端：修改数据
void update_data(struct double_buffer *db) {
    int write_index = 1 - db->active_index;  // 写入非活跃缓冲区
    prepare_data(db->buffer[write_index]);
    __sync_synchronize();
    __sync_lock_test_and_set(&db->active_index, write_index);  // 切换缓冲区
}

// 发起端：读取数据
void read_data(struct double_buffer *db) {
    int read_index = db->active_index;  // 读取当前活跃缓冲区
    rdma_read(db->buffer[read_index]);   // RDMA Read
    // 即使对端切换缓冲区，读取的也是完整的数据
}
```

#### 同步机制选择建议

| 场景 | 推荐机制 | 原因 |
|:---:|:---|:---|
| **需要通知对端数据已写入** | RDMA Write with Immediate | 高性能 + 通知 |
| **需要严格顺序** | Fence + Send/Recv | 保证操作顺序 |
| **需要检测数据变化** | 版本号机制 | 可以检测并发修改 |
| **需要无锁读取** | 双缓冲机制 | 避免锁竞争 |
| **需要细粒度控制** | 原子标志位 | 精确控制读写时机 |

## 应用与实践

### 应用场景对比

| 场景类型 | Send/Recv | Read/Write |
|:---:|:---------:|:----------:|
| **请求-响应模式** | ✅ 适合（RPC、数据库查询） | ❌ 不适合 |
| **需要流控** | ✅ 适合（接收端处理能力有限） | ❌ 不适合 |
| **需要确认** | ✅ 适合（事务提交、状态同步） | ❌ 不适合 |
| **小数据频繁交互** | ✅ 适合 | ❌ 不适合 |
| **协议实现** | ✅ 适合（分布式一致性协议） | ❌ 不适合 |
| **高性能计算（HPC）** | ❌ 不适合 | ✅ 适合（科学计算、数值模拟） |
| **AI 训练** | ❌ 不适合 | ✅ 适合（参数同步，对端 CPU 繁忙） |
| **存储系统** | ❌ 不适合 | ✅ 适合（块存储、文件系统） |
| **批量数据传输** | ❌ 不适合 | ✅ 适合（大数据传输） |
| **零拷贝需求** | ❌ 不适合 | ✅ 适合（避免 CPU 参与） |

### 混合使用策略

在实际应用中，可以混合使用两种模式：

```c
// 示例：使用 Send/Recv 进行控制，使用 Write 进行数据传输

// 1. 通过 Send/Recv 交换元数据
struct metadata {
    uint64_t data_addr;
    uint32_t data_rkey;
    size_t data_size;
} meta;
send_metadata(&meta);

// 2. 通过 RDMA Write 传输实际数据
rdma_write_data(meta.data_addr, meta.data_rkey, data_buffer, meta.data_size);

// 3. 通过 Send/Recv 发送完成通知
send_completion_notification();
```

| 用途 | 使用的模式 | 原因 |
|:---:|:---|:---|
| 元数据交换 | Send/Recv | 需要确认和流控 |
| 数据传输 | Read/Write | 需要最高性能 |
| 完成通知 | Send/Recv | 需要确认 |

### 性能优化

| 优化策略 | Send/Recv | Read/Write |
|:---:|:---------|:----------|
| **批量操作** | 批量提交 Receive WR | 批量提交 Read/Write WR |
| **Signaling 策略** | 周期性 Signaling（每 N 个请求） | 避免频繁 Signaling |
| **内存对齐** | 建议对齐 | 强烈建议对齐（64 字节） |
| **队列大小** | 合理设置 max_recv_wr | 合理设置 max_send_wr |
| **RNR 参数** | 合理设置 rnr_retry 和 min_rnr_timer | 不适用 |

**Send/Recv 性能优化代码**：

```c
// 1. 批量提交 Receive WR
for (int i = 0; i < BATCH_SIZE; i++) {
    post_recv_wr();
}

// 2. 周期性 Signaling
if (count % 64 == 0) {
    send_wr.send_flags |= IBV_SEND_SIGNALED;
}
```

**Read/Write 性能优化代码**：

```c
// 1. 批量操作
struct ibv_send_wr *wr_list = build_wr_list();
ibv_post_send(qp, wr_list, &bad_wr);

// 2. 避免频繁 Signaling
write_wr.send_flags = 0;  // 不请求完成通知

// 3. 内存对齐
posix_memalign((void**)&buffer, 64, size);  // 64 字节对齐
```

### 注意事项

| 注意事项 | Send/Recv | Read/Write |
|:---:|:---------|:----------|
| **必须预先 post recv** | ✅ 必须 | ❌ 不需要 |
| **及时处理 CQ 事件** | ✅ 必须（避免 CQ 溢出） | ✅ 必须（仅发起端） |
| **合理设置队列大小** | ✅ 重要 | ✅ 重要 |
| **内存访问权限** | ✅ 需要 | ✅ 需要（对端需设置 REMOTE 权限） |
| **R_Key 安全** | ❌ 不需要 | ✅ 重要（需要安全传递） |
| **同步机制** | ✅ 通过 CQ 事件 | ✅ 需要额外机制 |
| **内存边界检查** | ✅ 需要 | ✅ 需要（避免越界访问） |
| **并发安全** | ✅ 需要应用层同步 | ✅ 需要应用层同步 |

## 总结

Send/Recv 和 Read/Write 代表了 InfiniBand/RDMA 的两种不同设计哲学：

| 模式 | 设计哲学 | 核心特点 |
|:---:|:---|:---|
| **Send/Recv** | 协作和流控 | 接收端参与，有流控机制，适合需要确认的场景 |
| **Read/Write** | 极致性能和 CPU 卸载 | 对端 CPU 不参与，无流控，适合高性能场景 |

在实际应用中，应根据具体场景的需求，灵活选择或组合使用这两种模式，充分发挥 InfiniBand/RDMA 的性能优势。
