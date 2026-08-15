---
title: MIT 6.004：L19 并发与同步
date: 2026-08-11 10:19:00
categories: ic
tags:
  - ic
  - digital-circuit
  - concurrent
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L19 注解幻灯片。
>
> 源网页：[19.1 Annotated Slides | Concurrency and Synchronization](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c19/c19s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L19：并发与同步（Concurrency and Synchronization）

本讲以 **producer–consumer** 为线索，引入 precedence constraint、FIFO 缓冲，以及 Dijkstra 的 **semaphore**（WAIT/SIGNAL）；覆盖资源分配、互斥临界区、多生产者/消费者，以及用 SVC 或 **TCLR**（test-and-clear）实现 semaphore；最后讨论 **deadlock**（Dining Philosophers）与全局资源序 / 检测恢复。

## 1. 进程间通信（Interprocess Communication）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/eb4084d127d79b1af9f75365ff615bd9_Slide02.png" alt="Interprocess Communication" width="70%"/>

应用常拆成多进程：视频压缩可并行处理宏块；游戏分前端 UI 与后端仿真/渲染。进程封装独立状态，需要时再共享信息。

通信方式：

- **共享内存**：同一物理页映射进两进程；配合同步原语（部分 ISA 有专用指令）
- **消息传递**：经 OS SVC；开销更大，但编程模型不依赖是否同机

本讲用经典 **producer–consumer** 作并发同步范例。

## 2. 同步通信（Synchronous Communication）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3fcd6b0266322bfe94c8e661a0b720b2_Slide03.png" alt="Synchronous Communication" width="70%"/>

单进程内程序计数器决定执行顺序。跨进程还需 **precedence constraints**（记号 $\prec$）：第 $i$ 次 send 须先于第 $i$ 次 receive；若共享单单元，第 $i$ 次 receive 须先于第 $i+1$ 次 send（防覆盖）。二者使产消**紧耦合**——消费完才能再生产。下一节用缓冲放松约束。

## 3. FIFO 缓冲（FIFO Buffering）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/cbb5722d9bbf949e6dba2a5f1f6afb86_Slide04.png" alt="FIFO Buffering" width="70%"/>

$N$ 字符 **FIFO**：空则消费者等，满则生产者等。覆盖约束放宽为：第 $i$ 次 receive $\prec$ 第 $i+N$ 次 send——生产者最多超前 $N$ 个。

实现：长度 $N$ 的环形数组 + 读/写下标（模 $N$ 递增）；另需计数（图中略）。任意交错执行皆可，只要不满写、不空读。

## 4. 例：有界缓冲问题（Example: Bounded Buffer Problem）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7f17e586f3fa2e9cef5a958bf2c5643e_Slide05.png" alt="Bounded Buffer Problem" width="70%"/>

共享数组与 `IN`/`OUT`。`SEND` 用 `IN` 写，`RCV` 用 `OUT` 读，各自模 $N$ 递增。如图代码**未强制**任何 precedence——可空读、可满写。需要同步抽象。

## 5. 信号量（Dijkstra）（Semaphores (Dijkstra)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/782c2bd2fdb2289ea369fc01c2e12a18_Slide06.png" alt="Semaphores" width="70%"/>

Dijkstra 提出 **semaphore**：共享整数 ≥0；操作：

- **WAIT(s)**：等 $s>0$ 再减一返回（实现上可忙等或挂起）
- **SIGNAL(s)**：加一；若有等待者，**恰好一个**可继续

初值 $K$ 保证：第 $i$ 次 SIGNAL $\prec$ 第 $i+K$ 次 WAIT 完成。本课不允许负值。文献亦称 P/V（荷兰语“测/增”）。

## 6. 用信号量表达先后（Semaphores for Precedence）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c7b1ead2c4b41f946a2fcf649fcb15a8_Slide07.png" alt="Semaphores for Precedence" width="70%"/>

指南：semaphore 初值 0；箭头**起点**后放 `signal(s)`，**终点**前放 `wait(s)`。例：A2 后 signal、B4 前 wait → 保证 A2 完成才开始 B4。初值 0 强制第一次 signal 先于第一次 wait。

## 7. 用信号量做资源分配（Semaphores for Resource Allocation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2e0ee81b0052ac4e534b604dd8862c84_Slide08.png" alt="Semaphores for Resource Allocation" width="70%"/>

另一视角：初值 $K$ = 共享资源池大小。SIGNAL 归还/加入资源，WAIT 独占领取；当前值 = 剩余未分配数。WAIT/SIGNAL 可同进程或跨进程。

## 8. 有界缓冲 + 信号量（Bounded Buffer Problem with Semaphores）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/466700fb4097b8f26bf717610c6d32b1_Slide09.png" alt="Bounded Buffer with Semaphores" width="70%"/>

`CHARS` 初值 0：SEND 写入后 `signal(CHARS)`；RCV 先 `wait(CHARS)` 再读。保证消费者不空读。但只实现了两条 precedence 中的一条。

## 9. 流控问题（Flow Control Problems）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/69cf34264659c1247eb3a74cea16bdd9_Slide10.png" alt="Flow Control Problems" width="70%"/>

仅有 `CHARS` 时，生产者仍可写入超过 $N$ 个 → **buffer overflow**，字符流损坏。还需：第 $i+N$ 次 send 之前必须完成第 $i$ 次 receive。

## 10. 更多信号量的有界缓冲（Bounded Buffer Problem with More Semaphores）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c01bf6d559a14c22b299330837470cab_Slide11.png" alt="Bounded Buffer More Semaphores" width="70%"/>

加 `SPACES` 初值 $N$：生产者 `wait(SPACES)` 再写；消费者读后 `signal(SPACES)`。对称：生产者消费空位、生产字符；消费者反之。单生产者+单消费者至此正确；多对多另有问题。

## 11. 同时事务（Simultaneous Transactions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4d090643f7efaaef7818c2e9b68614e6_Slide12.png" alt="Simultaneous Transactions" width="70%"/>

两客户同时从同一账户取 \$50。若两次 `Debit` 完整串行执行：余额减 \$100，正确。

## 12. 但若…（But, What If…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c4791bbd00a4b82ba0372f71f16b8827_Slide13.png" alt="But What If" width="70%"/>

进程 A 读完余额后被打断，B 完成扣款，A 用过期余额写回 → 只扣了 \$50。共享数据上的 LD/修改/ST 构成 **critical section**，需要 **mutual exclusion**：同时只有一个进程在临界区内。Semaphore + 临界区 ≈ **transaction**（期间共享数据不被他进程读写）。

## 13. 互斥用信号量（Semaphores for Mutual Exclusion）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7ba6611519d0b6eb9579af3331aec584_Slide14.png" alt="Semaphores for Mutual Exclusion" width="70%"/>

`LOCK` 初值 1：进临界区前 WAIT（acquire），出后 SIGNAL（release）。锁的**粒度**重要：全行一个锁会串行化无关账户；**每账户一锁**只阻塞真正冲突的事务，吞吐更好。

## 14. 产消原子性问题（Producer/Consumer Atomicity Problems）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a9b2e2d16758fa0c0e3fa62a4b217399_Slide15.png" alt="Producer Consumer Atomicity" width="70%"/>

多生产者同时插入时，对 FIFO/`IN` 的更新可能交错 → 覆盖或下标错误。插入路径是临界区，须原子执行。

## 15. 再加信号量的有界缓冲（Bounded Buffer Problem with Even More Semaphores）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/357dd1158430cee1ec864ce10b2ac60a_Slide16.png" alt="Bounded Buffer Even More Semaphores" width="70%"/>

第三 semaphore `LOCK` 保护 SEND/RCV 中操作缓冲的临界区。同锁可用于多消费者，但生产者用 `IN`、消费者用 `OUT`，共用一把锁引入多余先后约束——可拆成两把锁。

## 16. 信号量的威力（The Power of Semaphores）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4e2f8ef94bb743f997dab9e20ccd307e_Slide17.png" alt="The Power of Semaphores" width="70%"/>

Semaphore 像瑞士军刀：跨进程 WAIT/SIGNAL 保证时序（空不读、满不写）；同进程内可实现临界区原子性（如 `IN`/`OUT` 的读改写不被打断）。

## 17. 信号量实现（Semaphore Implementation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c173dceca094760016349a4ce797ccda_Slide18.png" alt="Semaphore Implementation" width="70%"/>

Semaphore 自身是共享数据，WAIT/SIGNAL 的读改写也是临界区——不能再用 semaphore 实现 semaphore（**bootstrap**）。出路：

1. 不可中断内核上的 **SVC**
2. ISA 的 **test-and-set / test-and-clear**，由内存原子读改写支持
3. 纯软件算法（如 Dekker）仅依赖单次读写原子性

本讲详述前两种。

## 18. 作为 SVC 的信号量（Semaphores as a Supervisor Call）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9059d77ae437a6f04313f507ce660af0_Slide19.png" alt="Semaphores as SVC" width="70%"/>

内核态 SVC 不可中断 → handler 天然临界区。WAIT：值非零则减一并返回；为零则安排重试 SVC 并 `SLEEP`。SIGNAL：加一并 `WAKEUP` 等该 semaphore 的进程。

默认实现**无公平性**——调度顺序决定谁先拿到；若要公平，WAIT 可维护等待队列。

## 19. 硬件支持（Hardware Support for Semaphores）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7ed986d4ea119554ac7686769dcb97e6_Slide20.png" alt="Hardware Support for Semaphores" width="70%"/>

**TCLR**（test-and-clear）：一次原子操作读内存当前值并清零。自旋：TCLR 得 0 → 别人持锁，重试；得非 0 → 已获取锁（且已把锁置 0）。临界区结束用 ST 写回非 0 释放。

## 20. 同步的阴暗面（Synchronization: The Dark Side）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9664c5191d8248fd1117914449a9219a_Slide21.png" alt="Synchronization Dark Side" width="70%"/>

转账需拿两账户锁。两人按**相反顺序**各拿到第一把锁后，都等对方释放第二把 → **deadlock（deadly embrace）**。多资源同步需额外纪律。

## 21. 哲学家就餐（Dining Philosophers）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b1ec628d898f7ccfb1b7276925dd6f8f_Slide22.png" alt="Dining Philosophers" width="70%"/>

5 哲学家、5 筷；每人需左右两筷。算法：先左后右，吃完归还。典型的“多资源才可完成”设定。

## 22. 死锁！（Deadlock!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d3261c81a0a0f2696f5691f10d628d5d_Slide23.png" alt="Deadlock" width="70%"/>

若人人先拿左筷，则无人能拿右筷 → 死锁。四条件：

1. **Mutual exclusion**
2. **Hold-and-wait**
3. **No preemption**
4. **Circular wait**

对策：**避免**，或**检测 + 恢复**。

## 23. 一种解法（One Solution）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5aacddd6c3a3182ff8945ec7b796e6b9_Slide24.png" alt="One Solution" width="70%"/>

给筷子唯一编号，**按全局序**取资源（先低号再高号）。若全部筷子都被拿起，必有人已持有最高号筷，此前也已拿到另一侧低号筷 → 此人可吃并归还，打破 hold-and-wait 环。全系统约定资源全局序并按序获取 → 无环等待死锁。

## 24. 处理死锁（Dealing With Deadlocks）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a897d8e1eb4d36deea89b12ad008a9ee_Slide25.png" alt="Dealing With Deadlocks" width="70%"/>

转账：先锁低账号，再锁高账号——双方先争同一把“第一资源”，胜者可安全拿齐其余。无法改应用时：OS 的 WAIT 可检测环等待并终止一进程释放资源；数据库则检测冲突、**abort** 事务并由程序员决定重试，提交前改动只在事务私有副本上，确认后才 **commit**。

## 25. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/73a898d12dac31e50cc7be6ed22195ef_Slide26.png" alt="Summary" width="70%"/>

- 多进程组织应用常更自然；用 **semaphore** 保证 precedence 与 **mutual exclusion**
- 临界区 + 锁实现事务语义
- 多锁可能 **deadlock**；全局资源序避免，或检测/重启恢复
- 大数据与云上千进程协作时，同步是核心技能
