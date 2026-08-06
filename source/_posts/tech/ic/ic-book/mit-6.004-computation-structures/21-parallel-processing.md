---
title: MIT 6.004：L21 并行处理
date: 2026-08-11 10:21:00
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L21 注解幻灯片。
>
> 源网页：[21.1 Annotated Slides | Parallel Processing](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c21/c21s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L21：并行处理（Parallel Processing）

本讲从单核性能公式出发，讨论更深/更宽流水线、**ILP**、乱序超标量的极限；再转向 **DLP**（向量/GPU）与 **TLP**（多核）；用 **Amdahl’s Law** 框定加速比；最后讲共享内存多核的缓存一致性：**sequential consistency**、**MESI** snoopy 协议与 barrier。

## 1. 处理器性能（Processor Performance）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4fa6689b241cc8af45c2d83d7e0d79ed_Slide02.png" alt="Processor Performance" width="80%"/>

程序运行时间 = 指令数 × 平均每指令周期（CPI）× 时钟周期 $t_{\mathrm{CLK}}$。指令数由 ISA/编译器决定；本讲聚焦后两项。流水线减小 $t_{\mathrm{CLK}}$。理想 5 级 Beta 每周期完成 1 条 → $\mathrm{CPI}_{\mathrm{ideal}}=1$，但分支、紧接使用 LD、cache miss 引入 NOP bubble → $\mathrm{CPI}_{\mathrm{stall}}$。

## 2. 五级流水线处理器（5-Stage Pipelined Processors）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/216bc93a4d52cfcae4fe6ffd0a6c26f1_Slide03.png" alt="5-Stage Pipelined Processors" width="80%"/>

经典 5 级是 $t_{\mathrm{CLK}}$ 与 $\mathrm{CPI}_{\mathrm{stall}}$ 的折中。局限：每级同时只处理一条 → $\mathrm{CPI}_{\mathrm{ideal}}=1$；慢操作（乘、大 cache）迫使 $t_{\mathrm{CLK}}$ 变长；流水线内指令顺序固定——LD 在 MEM 因 miss 停住时，前面无关指令也被拖住。如何放松这些约束？

## 3. 改进五级流水线性能（Improving 5-Stage Pipeline Performance）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bf0b4e1e674f93072d91fe5a8171c578_Slide04.png" alt="Improving Pipeline Performance" width="80%"/>

加深流水：拆瓶颈（如 MEM1/MEM2），可缩短时钟，但 LD 数据冒险需更多 bubble → $\mathrm{CPI}_{\mathrm{stall}}$ 升。更深意味着更多指令并行执行。

## 4. 流水线深度的极限（Limits to Pipeline Depth）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bfcbaf2a14b7612e57b1c18a7699afde_Slide05.png" alt="Limits to Pipeline Depth" width="80%"/>

每级额外开销 $O$：寄存器 $t_{\mathrm{PD}}$/setup/hold、**clock skew**、工作量不均的浪费。原周期 $T$、$N$ 级后周期 $\approx T/N+O$；大 $N$ 时加速比逼近 $T/O$——开销主导。Intel Core-2（Nehalem）约 **14 级**执行流水。

## 5. 继续改进清单（Improving 5-Stage Pipeline Performance）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dc1a564276983b421b46f9fad130d822_Slide06.png" alt="Improving Pipeline Performance 2" width="80%"/>

- **多发射**：独立指令并行 → 提高 $\mathrm{CPI}_{\mathrm{ideal}}$，级更复杂
- **Out-of-order**：冒险阻塞时允许后续无关指令越过
- 更深更宽放大控制冒险代价 → 需 **branch prediction** 降低 $\mathrm{CPI}_{\mathrm{stall}}$

可并行/可重排的指令量合称 **instruction-level parallelism（ILP）**。

## 6. 指令级并行（Instruction-level Parallelism (ILP)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6ae04ef99d6301923171df7deaa12424_Slide07.png" alt="Instruction-level Parallelism" width="80%"/>

阶乘循环例：同行可并发；BF 下方指令仅在未跳转时有效（可投机执行但须能丢弃）。约束：

- **RAW**（红）：读依赖先前写——旁路可解，但仍需产生结果的指令先执行
- **WAW**（绿）、**WAR**（蓝）：可用寄存器重命名消除

本例中 BF 后潜在并发其实不少。

## 7. 更宽或超标量流水线（Wider or Superscalar Pipelines）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7e78206cca5f71ea2442f8af7c593497_Slide08.png" alt="Wider or Superscalar" width="80%"/>

并行执行 $N$ 条时，$\mathrm{CPI}_{\mathrm{ideal}}=1/N$。不同功能单元（ADD/SHIFT、整/浮点、LD/ST 地址单元）易并行；多加法器、多端口寄存器堆/内存支撑并发。Nehalem 每周期最多完成约 **4** 个 micro-op（≈ 简单 RISC 指令）。

## 8. 现代乱序超标量（A Modern Out-of-Order Superscalar Processor）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ac44973487c072199085e2589d7887dc_Slide09.png" alt="Modern OoO Superscalar" width="80%"/>

取指/译码一次多条；强依赖分支预测。译码时 **register renaming**，再派发到功能单元队列，操作数齐则执行——顺序可不同于程序序。结果广播唤醒等待者，并进 **reorder buffer** 按正确顺序 retire。电路量大；相对单发按序，平均加速约 **2×**（理想上界约 4）。

## 9. 单处理器性能极限（Limits to Single-Processor Performance）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fe9f32bf6d27d6c81e454294f41f5b61_Slide10.png" alt="Limits to Single-Processor Performance" width="80%"/>

再加深：$t_{\mathrm{CLK}}$ 收益不抵 $\mathrm{CPI}_{\mathrm{stall}}$/开销上升；再加宽乱序亦然；功耗涨得比性能快；分支预测与并发硬件愈发复杂。结论：乱序超标量难再大幅跃进 → 转向 **DLP** 与 **TLP**。

## 10. 数据级并行（Data-Level Parallelism）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/346dc184db3dd1af572b42fb108dd96f_Slide11.png" alt="Data-Level Parallelism" width="80%"/>

音频向量、图像像素矩阵常对每元素做相同运算。复制 datapath，共享译码控制 → **向量处理器**：块取存（似 cache line），总线一次送多字。一条指令 ≈ 标量机 $N$ 条，并行性“编进”程序，无需乱序发现。

## 11. 向量代码例（Vector Code Example）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bf36d63f8f5354f4a652be5dc21fe6b1_Slide12.png" alt="Vector Code Example" width="80%"/>

16 路向量机做向量加：Beta 循环约 9 指令/10 周期 ×16 ≈ **160** 周期；向量码约 **4** 周期 → 加速约 40（理想情况）。关键是能否 **vectorize**；音视频与 DSP 通常可以。内存块访问也摊薄开销。

## 12. 数据相关的向量操作（Data-Dependent Vector Operations）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/aa2d3aa2765e61458b88ba48ca319557_Slide13.png" alt="Data-Dependent Vector Operations" width="80%"/>

条件执行：各 datapath 设本地 **predicate**；`CMPLT.V` 并行比较并置谓词；`ADDC.V.iftrue` 仅谓词为真时执行。**Predication** 在非向量 ISA 也用于避免短条件分支的误预测代价（x86 CMOV、ARM 条件执行）。

## 13. 向量处理实现（Vector Processing Implementations）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6d1d800cc028ad069550cc1a2e0362b9_Slide14.png" alt="Vector Processing Implementations" width="80%"/>

现代 CPU 常有 SIMD/向量扩展（128/256/512-bit 打包 8–64-bit 元素）。**GPU** 是极端多 datapath，专长 3D 渲染中“尴尬并行”的浮点变换/着色/纹理；亦用于生物信息、大数据、深度学习等。DLP 在多种场景显著加速，未来 ISA 几乎都会保留向量支持。

## 14. 多核处理器（Multicore Processors）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/361e731015aae8599309a226b9dcbfe7_Slide15.png" alt="Multicore Processors" width="80%"/>

单核成本–性能曲线陡：半性能可能只需 1/4 成本。任务可拆成独立子任务时，多个更小核可达相近总性能且更便宜；并行可扩展时性能近似随核数线性。最优点数受分发/聚合开销制约，但“更多更高效小核”仍有吸引力。

## 15. Amdahl 定律（Amdahl’s Law）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7a4415bef593b6e79608a078d2e9ebf8_Slide16.png" alt="Amdahl's Law" width="80%"/>

Gene Amdahl（1967）：加速任务中比例 $F$ 的部分 $S$ 倍，整体加速比 $=1\big/\big((1-F)+F/S\big)$。应优先加速占比大的部分（做大 $F$）。

## 16. Amdahl 与并行（Amdahl’s Law and Parallelism）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/433acfd2c5ba799e0e33d6c0ab07d64d_Slide17.png" alt="Amdahl's Law and Parallelism" width="80%"/>

并行部分 $F$ 可任意加速时，整体加速上界 **$1/(1-F)$**。90% 可并行 → 最多 10×；想在 1000 核上拿 500×，需并行化约 **99.8%**。多核最适合天然高并行任务。

## 17. 线程级并行（Thread-Level Parallelism）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5076f21bab3a48e9377f49ed513055db_Slide18.png" alt="Thread-Level Parallelism" width="80%"/>

**TLP**：每核跑独立线程，比向量的锁步更灵活。少核时常 **共享内存** 通信；数十/数百核则共享内存带宽成瓶颈，改用消息网络（片上 mesh、集群 **MPI** / InfiniBand）。以下聚焦共享内存多核问题。

## 18. 多核缓存（Multicore Caches）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/15822690d051d32992af3a69bc9ce215_Slide19.png" alt="Multicore Caches" width="80%"/>

每核私有 cache（写回）降低平均访存；miss 才打共享主存。目标：一核对共享变量的修改应对所有核可见。例：核 0/1 各跑线程 A/B，共享 $X=1$、$Y=2$，并已缓存在两核。

## 19. 可能的结果？（What Are the Possible Outcomes）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/53636f49d925260d2005c7a8cbf290f8_Slide20.png" alt="What Are the Possible Outcomes" width="80%"/>

各线程只更新本地 cache：打印结果可一致（A 打 2、B 打 1），但结束后两核对 $X,Y$ 的缓存副本可分歧——**不再像单一共享内存**。去掉 cache 又会毁掉多核性能。

## 20. 单处理器结果（Uniprocessor Outcome）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fd9844f75b4e78788e7195a70e7b8c5c_Slide21.png" alt="Uniprocessor Outcome" width="80%"/>

正确性基准：同一分时单核上交错执行的可能结果集。程序员知结果不唯一，需用 semaphore 等加约束。简单多核出现的 (2,1) **不在**该交错结果表中。

## 21. 顺序一致性（Sequential Consistency）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/125d2f5d9315abd8444acfad1e91d829_Slide22.png" alt="Sequential Consistency" width="80%"/>

**Sequential consistency**：并行执行 $N$ 线程 ≡ 某次单核交错。简单多核两败：① 共享变量副本不一致；② 因而也不满足顺序一致性。需要修复。

## 22. 顺序一致性的替代？（Alternatives to Sequential Consistency?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e67e1375d3dea08228434cd75d7b7fed_Slide23.png" alt="Alternatives to Sequential Consistency" width="80%"/>

**Weak consistency**：只保证单线程内发出的访存按该线程顺序对外可见（写 X 再写 Y → 无人会看到新 Y 旧 X）；他线程操作可任意重叠。私有写回 cache 连弱一致性也不自动保证（脏 Y 可能先于脏 X 写回）。乱序核提供 **BARRIER**：屏障前访存完成才执行屏障后访存。各商用多核语义各异——须读 ISA 手册。

## 23. 修复：侦听缓存一致性（Fix: “Snoopy” Cache Coherence Protocol）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8fbd7d6aa102e09844d7602bed94f0c4_Slide24.png" alt="Snoopy Cache Coherence" width="80%"/>

缺通信：改共享变量时他核不知。在共享总线上让各 cache **snoop**，更新本地状态 → **cache coherence protocol**。希望仅在真正共享时才付通信开销。

## 24. 例：MESI 协议（Example: MESI Cache Coherence Protocol）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fed65c0750030cacfd0d3affc2f9f9dd_Slide25.png" alt="MESI Protocol" width="80%"/>

每 cache line 状态：

| 状态 | 含义 |
|------|------|
| **I**nvalid | 无效（原 valid=0） |
| **E**xclusive | 独有且与主存一致 |
| **M**odified | 独有且已改（脏） |
| **S**hared | 多副本，未改 |

读 miss：无他核 → 自内存装入置 E；他核有 E/S → 供应数据并标 S；他核有 M → 供应脏数据并写回内存，双方标 S。写：若 S 则发 **INVALIDATE** 使他核失效后独占再改；已是 E 则可本地改而无广播。

## 25. Cache 有两位“顾客”！（The Cache Has Two Customers!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6346d029b96e010bab921d7dfc90c894_Slide26.png" alt="Cache Has Two Customers" width="80%"/>

硬件服务两路请求：CPU 侧（含 store 队列，miss 时 CPU 可继续；读须先看 store 队列）与 snoopy 总线侧（失效/供应/改状态）。**STORE_BARRIER** 等到 store 队列空；**READ_BARRIER** 等到 invalidate 队列空。“read with intent to modify” = READ 紧接 INVALIDATE。

## 26. MESI 活动图（MESI Activity Diagram）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/391aefa6a5c2ba716e74881a69548e78_Slide27.png" alt="MESI Activity Diagram" width="80%"/>

流程图（字极小）给出 CPU/总线事务如何改状态。Intel 另加 **F** 态，在多份 SHARED 中指定谁响应读请求。下面用前述例子走一遍 MESI。

## 27. 缓存一致性实战（Cache Coherence in Action）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8d5fb46ab0947c2fae900560b5fa25a6_Slide28.png" alt="Cache Coherence in Action" width="80%"/>

$X,Y$ 初始 SHARED。顺序 (1)–(4)：

1. A 写 $X=3$：发 INVALIDATE → 独占改写；核 1 失去 $X$
2. B 写 $Y=4$：同理独占 $Y$
3. B 读 $X$：miss → 核 0 供应新值，双方标 S，主存更新
4. A 读 $Y$：对称事务

结果与同序单核交错一致；两核最终对 $X,Y$ 看法一致。其他交错同样保持顺序一致性与共享语义——一致性协议尽责。

## 28. 并行处理小结（Parallel Processing Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9eaf22da9c5e9d4879085db415b1824d_Slide29.png" alt="Parallel Processing Summary" width="80%"/>

单核流水深度与乱序超标量已近收益递减；GPU 继续为专用负载演进，但不取代通用计算。系统趋势是**更多核** + 新算法挖并行。展望：大脑用很慢的机制完成非凡认知——靠大规模并行，还是不同计算模型（如神经网络）？认知类应用仍有架构与技术新边疆。
