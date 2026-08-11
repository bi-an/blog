---
title: MIT 6.004：L14 高速缓存与存储层次
date: 2026-08-11 17:37:00
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L14 注解幻灯片。
>
> 源网页：[14.1 Annotated Slides | Caches and the Memory Hierarchy](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c14/c14s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L14：高速缓存与存储层次（Caches and the Memory Hierarchy）

本讲从“Beta 其实是内存机”出发，纵览 SRAM/DRAM/Flash/硬盘，引入**局部性**与隐藏的存储层次；再系统讲 cache 命中率与 AMAT、直接映射 / 全相联 / $N$ 路组相联、块大小、替换与写回策略。

## 1. 我们的内存机（Our Memory Machine）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d2ef45f23af942263e574a18eba188e1_Slide02.png" alt="Our Memory Machine" width="80%"/>

Beta 结构简单，但三端口主存在面积与周期占比上往往最贵——更像“内存机”而非“计算机”。每条指令先取指；数据最终都经主存进出；寄存器只能留极少热数据。现代机性能常受 CPU↔主存带宽（**memory bottleneck**）限制。本讲目标：理解瓶颈并尽量用体系结构缓解。

## 2. 存储技术（Memory Technologies）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3f4271fe83edb40d1897b8c2a415c2d3_Slide03.png" alt="Memory Technologies" width="80%"/>

| 技术 | 特点（量级） |
|------|----------------|
| 寄存器（时序逻辑） | 延迟极低（~20 ps），容量千比特级 |
| SRAM | 低延迟（ns 级），数千～更多单元 |
| DRAM | 大容量、低成本，延迟更长 |
| Flash / HDD | 非易失；HDD 最底层、极大且便宜 |

容量↑ → 面积↑ → 线长/电容↑ → 更慢：根本的尺寸–性能权衡。将用 SRAM+DRAM 建层次，追求**低平均延迟**与高容量（依赖访问统计，最坏情况仍可能慢）。Flash 相对 HDD 类似 SRAM 相对 DRAM。

## 3. 静态 RAM（Static RAM）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2272a809e48f97268ed5026d413b1de6_Slide04.png" alt="Static RAM SRAM" width="80%"/>

SRAM：按地址读写一整行（一个 location）。例：8 行 × 6 列 → 需 3 位地址。译码器拉高一条 **wordline** 选中一行；该行各单元接到垂直 **bitline**；读时 sense amp 把模拟差转为数字，写时驱动器把数据打上 bitline。大容量 SRAM 会组织得更复杂以缩短 bitline。

## 4. SRAM 单元（SRAM Cell）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1ab291b0a994972987e9edda355b24df_Slide05.png" alt="SRAM Cell" width="80%"/>

典型 6T 单元：两 CMOS 反相器正反馈形成双稳态；两侧经 access FET 接一对 bitline。wordline 高 → 接通；低 → 与 bitline 隔离，有电即可保持。

## 5. SRAM 读（SRAM Read）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f4620068350644ca5c6a660d869f457f_Slide06.png" alt="SRAM Read" width="80%"/>

先预充 bitline 到 VDD 再浮空；拉高 wordline 后小尺寸反相器缓慢拉低一侧 bitline。Sense amp 检测微小差分电压即出数字结果——读本质是模拟；差分 + 双 bitline 提高抗噪。

## 6. SRAM 写（SRAM Write）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/eba17048ae25f96b2186907043c3fce8_Slide07.png" alt="SRAM Write" width="80%"/>

先把 bitline 驱动到目标值，再开 wordline；大驱动管压过单元内小逆变器，双稳态翻转到新态。几乎由接 0 的大 nFET 下拉完成（过功率小 pFET）。尺寸须仔细平衡以保证快且可靠——亦是模拟操作。

## 7. 多端口 SRAM（Multiported SRAMs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c7c53cab6c148d5ad05e42e5273cf3ee_Slide08.png" alt="Multiported SRAMs" width="80%"/>

加套 wordline/bitline/驱动/sense → 多独立端口（寄存器堆常用）。每 bit 需 $N$ 条 wordline、$2N$ 条 bitline、$2N$ 个 access FET；面积大致随端口数**平方**增长——勿滥加端口。

## 8. SRAM 小结（Summary: SRAM）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/df320d2d2994fc8a189ad90af6fb942f_Slide09.png" alt="Summary SRAM" width="80%"/>

阵列组织；双稳态存 1 bit；读写经 bitline 的模拟操作；每 bit 约 6 MOSFET。能否更少？

## 9. 1T 动态 RAM 单元（1T Dynamic RAM Cell）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/241f43aa94bad9a9b82a34d081a30485_Slide10.png" alt="1T Dynamic RAM Cell" width="80%"/>

至少 1 个 access FET；用电容电压表示 0/1 → **DRAM**。沟槽电容增大极板面积而不占单元面积。约比 SRAM 密 20×。电荷会漏（PN 结、亚阈导通）→ 须约每 10 ms **refresh**（读后写回）。

## 10. 1T DRAM 读写（1T DRAM Writes and Reads）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/90bf003f5a86a7e4b55316dc96b44ef4_Slide11.png" alt="1T DRAM Writes and Reads" width="80%"/>

写：开 access FET，经 bitline 充/放电。读：bitline 预充中间电压，电荷共享导致微小电压变化，sense amp 检测；读破坏性 → 须写回。常按**行**（row address）宽读，再用**列**地址选字；同行后续列访问很快（fast column access）。

## 11. DRAM 小结（Summary: DRAM）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6f8f4ceafb94c0fcb7e94af6d7e5089e_Slide12.png" alt="Summary DRAM" width="80%"/>

1T+电容；读后重写 + 周期刷新；容量大、首访慢、同行后续快。断电丢数据 → 长期存储需非易失技术。

## 12. 非易失：Flash（Non-Volatile Storage: Flash）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e80a5998c3919b7871e7f30e77a4562a_Slide13.png" alt="Non-Volatile Storage Flash" width="80%"/>

电荷存在绝缘良好的**浮栅**上，可保持数年。有无电荷改变导通阈值；测电流甚至可多电平存多 bit。NOR 读延迟近 DRAM（数十 ns）；NAND 读更慢（~10 µs）；写需高压，慢，且擦写次数有限（$10^5\sim 10^6$）→ 片上地址重映射磨损均衡。相对 HDD：更快但更贵。

## 13. 非易失：硬盘（Non-Volatile Storage: Hard Disk）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0d170cf4b3a50a7e8e411becc4da00eb_Slide14.png" alt="Non-Volatile Storage Hard Disk" width="80%"/>

磁性盘片 5400–15000 RPM；磁头寻道 + 旋转等待，平均访问约 **10 ms**。就位后传输可达 ~100 MB/s；频繁寻道则有效速率骤降。TB 级廉价非易失，代价是慢。

## 14. 存储技术小结（Summary: Memory Technologies）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2bdbdcb3d5037e0ed2f1c87adf58eecb_Slide15.png" alt="Summary Memory Technologies" width="80%"/>

容量跨约 10 个数量级，延迟约 8 个。SRAM 跟得上工艺；DRAM/HDD 容量与带宽进步快，**首访延迟**进步慢；Flash 填补 CPU–HDD 间隙。每层都是：更小更快 vs 更大更慢——能否兼得？

## 15. 存储层次接口（Memory Hierarchy Interface）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0b9eb0f6769ae09bd9bd93e6ee4ca130_Slide16.png" alt="Memory Hierarchy Interface" width="80%"/>

理想：大、快、便宜的统一主存。单技术做不到 → 用不同权衡的层次：常访问数据放快层（SRAM），其余在慢大层，必要时搬移。

## 16. 存储层次接口（续）（Memory Hierarchy Interface continued）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8228765f1d48516a75931cb8759edd17_Slide17.png" alt="Memory Hierarchy Interface continued" width="80%"/>

两条路：

1. **暴露层次**：程序员显式搬数据（如 Seymour Cray 向量机）。
2. **隐藏层次**：给程序员平坦大地址空间；硬件按访问模式在层间自动搬移。

Cray 曾怀疑自动方案（“you can’t fake what you haven’t got”）。对通用程序，自动层次+局部性往往够好——引出 cache。

## 17. 局部性原理（The Locality Principle）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8764be3423356c8948d70e6606bd6e0a_Slide18.png" alt="The Locality Principle" width="80%"/>

希望把频繁访问数据放在快 SRAM，并预测将访问何处；一次搬入的块应被多次命中以摊销搬移开销。**局部性**（locality of reference）：时刻 $t$ 访问地址 $X$ → 不久很可能会访问**附近**地址。

## 18. 访存模式（Memory Reference Patterns）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4ae077a1fa40a1a84fc523dde47a31bf_Slide19.png" alt="Memory Reference Patterns" width="80%"/>

- **取指**：大多顺序；循环反复同一段；调用/分支打断后很快恢复顺序。过程入口后几乎会跑完整过程代码 → 整块搬入 SRAM 有利；DRAM 快列访问摊销首访。
- **栈帧**：过程期间密集访问小区域。
- **数据**：结构体字段、数组步进、区域间拷贝等也有局部性。

**工作集**（working set）：某时间窗内访问的不同地址数；窗变大后规模趋于平稳。

## 19. 高速缓存（Caches）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/cc46b2e5056d87ec7bbe7dbd5cb6ff3f_Slide20.png" alt="Caches" width="80%"/>

层次中靠近 CPU 的 SRAM 称 **cache**。命中（hit）由 SRAM 供数；缺失（miss）从 DRAM 搬入含该地址的块。局部性 ⇒ 命中远多于缺失。可有多级：近 CPU 更小更快，miss 查下一级。浏览器网页缓存是同思想。

## 20. 典型存储层次（A Typical Memory Hierarchy）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/49af1f412872ed7ec436623b7352a8f8_Slide21.png" alt="A Typical Memory Hierarchy" width="80%"/>

例：片上 L1/L2/L3 SRAM → DRAM 主存 → Flash 作 HDD 缓存。寄存器由编译器管理；片上 cache 与 DRAM 访问由硬件；更慢层常由软件管理。每层都试图对下一慢层的热数据提供更低延迟。

## 21. Cache 访问（Cache Access）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fb61248bca4643fd556daf48f2f057d1_Slide22.png" alt="Cache Access" width="80%"/>

CPU 发地址：命中则快返回；缺失则请主存、常把新数据写入 cache（可能替换旧块）。例：cache 4 ns、主存 40 ns → 命中 4 ns，缺失约 44 ns。CPU 须处理可变延迟（等待或切线程）。

## 22. Cache 指标（Cache Metrics）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f4884b1e9ce3f9e383724e3f01b27685_Slide23.png" alt="Cache Metrics" width="80%"/>

命中率 + 缺失率 $=1$。平均访存时间：

$$\mathrm{AMAT} = t_{\mathrm{hit}} + \mathrm{miss\_ratio}\times t_{\mathrm{miss\_penalty}}$$

每层可递归套用。更大更慢的下一级：hit time↑，但 miss ratio↓。

## 23. 例：命中率要多高？（Example: How High of a Hit Ratio?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ca88cd195076dca55d1b3e435f63408a_Slide24.png" alt="Example How High of a Hit Ratio" width="80%"/>

cache 4 周期、主存 100 周期：无 cache 恒 100；有 cache：命中 4、缺失 104。AMAT=100（打平）只需约 **4%** 命中率；目标 AMAT=5 则需约 **99%** 命中。SPEC CPU2000 上典型 L1 约 97.5%（约 $10^{13}$ 次访问）。

## 24. 基本 Cache 算法（Basic Cache Algorithm）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/461d3e1e649bb8e88684124ddd9f613c_Slide25.png" alt="Basic Cache Algorithm" width="80%"/>

每条 **cache line** = 数据块 + 地址 **tag**。按 tag 搜索：命中则读返回/写更新（并最终更新主存）；缺失则选一行替换，读则从主存填入，写则更新该行。内容由 CPU 请求塑造；工作集装得下 → AMAT 接近 hit time。关键：如何快速判断 tag 是否在某行。

## 25. 直接映射 Cache（Direct-Mapped Caches）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1acd90362aea5a2f7fa8acc4fae7c492_Slide26.png" alt="Direct-Mapped Caches" width="80%"/>

每个主存地址映射到**唯一**一行（**DM cache**）。用地址低部作 **index** 选一行，其余与该行 tag 比较；另有 **valid** 位（上电清 0）。字偏移：字寻址时低 2 位作 byte offset。index 取自低位地址位，使相邻地址映射到不同行，利于局部性。CPU 可 **flush** 使某些行无效（如 DMA 写入后）。

## 26. 直接映射示例（Example: Direct-Mapped Caches）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a496d57abf54435ee75001c8dafdec32_Slide27.png" alt="Example Direct-Mapped Caches" width="80%"/>

64 行 DM：地址拆成 offset / index / tag。例：0x400C → index=3、tag=0x40，行 3 tag 匹配 → 命中。0x4008 → index=2，tag 不匹配 → 缺失。由某行的 tag+index 可反推完整缓存地址（如 tag 同为 0x58 的行 0/1/2 → 0x5800/04/08）。

## 27. 块大小（Block Size）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bcc6b22ecceef24331cf5ecfe7fd20df_Slide28.png" alt="Block Size" width="80%"/>

每行可存 $2^k$ 个字（**block size**）。缺失时多取几个字，摊销 miss、提高后续命中；tag/valid 开销占比下降（例：4 字块 ~17% vs 1 字块 ~46% 开销）。整块要么全在要么全不在（单 valid）；局部性下通常值得整块装入。

## 28. 块大小权衡（Block Size Tradeoffs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ef68ae86c1a0e70711dcabf263e6fc99_Slide29.png" alt="Block Size Tradeoffs" width="80%"/>

块↑ → miss penalty 近似线性↑（DRAM 首访贵，后续列访问缓和）；miss ratio 先降。容量固定时块↑ ⇒ 行数↓ ⇒ 能同时容纳的独立地址区域变少，过大反伤工作集。存在最优块大小；现代常见 **64 B（16 字）**。用 AMAT 综合选取。

## 29. DM 的问题：冲突缺失（Direct-Mapped Cache Problem: Conflict Misses）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/10c62edb09ffbe02ae11a97e9200866b_Slide30.png" alt="Direct-Mapped Cache Problem Conflict Misses" width="80%"/>

代码与数据若映射到相同行，会互相踢出 → **conflict miss**，稳态命中率可从 100% 掉到 0%，程序突然慢一个数量级——破坏“平坦地址空间”抽象。需改进结构。

## 30. 全相联 Cache（Fully-Associative Cache）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c2deb76809aac914c2085b019791ae5e_Slide31.png" alt="Fully-Associative Cache" width="80%"/>

**FA**：每行都有比较器，并行比所有 tag → 任意块可进任意行，无地址冲突。灵活、命中率高；代价是比较器随行数线性涨（CAM 也难根本解决）。DM 只查 1 行，FA 查全部——中间地带？

## 31. N 路组相联 I（N-way Set-Associative Cache I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/de592696925a080bed7481cf437cb617_Slide32.png" alt="N-way Set-Associative Cache I" width="80%"/>

**$N$-way SA**：相当于 $N$ 个 DM 子 cache 并行。同一 index 的 $N$ 行组成一个 **set**；地址冲突最多可容 $N$ 个块。比较器只需 $N$ 个，可有大量行。在冲突敏感的 DM 与昂贵 FA 之间折中。

## 32. N 路组相联 II（N-way Set-Associative Cache II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3f10a3748f68bcf00624ea60812ca148_Slide33.png" alt="N-way Set-Associative Cache II" width="80%"/>

术语：**set** = 同 index 的 $N$ 行；**way** = 每个子 cache。路数不必是 2 的幂。管理保证同一地址不会出现在多 way（miss 时只写入一路）。

## 33. “数一数有几路”（Let me count the ways）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d13a6924bd57890b35b86761d169366a_Slide34.png" alt="Let me count the ways" width="80%"/>

所需路数大致对应时间窗内可能冲突的区域数（代码、栈、数据，拷贝时或需两块数据区）；大时间窗或再加倍。小数目的 way 通常足以消除绝大多数冲突。

## 34. 相联度权衡（Associativity Tradeoffs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/99ca3819302a4f5a3f9581304eff91aa_Slide35.png" alt="Associativity Tradeoffs" width="80%"/>

路数过多：合并 hit 信号的延迟抬高 hit time；miss ratio 在约 4–8 路后收益很小。大容量 8-way SA 常接近同容量 FA。

## 35. 相联意味着选择（Associativity Implies Choices）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0086cdc55f77d8e5db407f50f187c130_Slide36.png" alt="Associativity Implies Choices" width="80%"/>

Miss 时选哪一行替换？DM 无选择；SA/FA 有。目标：选对未来命中率损害最小的那一行。

## 36. 替换策略（Replacement Policies）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0bf2684b65cfc27714e5c3e42f5be8f4_Slide37.png" alt="Replacement Policies" width="80%"/>

最优：换掉最远将来才用（或永不用）的块——需预知未来。实用：**LRU**（least-recently-used）用过去近似未来。精确 LRU 状态大（8-way：$8!$ 序 → 约 16 bit）且更新逻辑贵 → 常用近似。其他：FIFO、随机。除随机外都可被恶意访问模式打穿；实践中 LRU/近似 LRU 仍合理。

## 37. 写策略（Write Policy）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/57848e2dd450f43e05bb08a0713681de_Slide38.png" alt="Write Policy" width="80%"/>

- **Write-through**：写 cache 同时写主存——主存始终新，但写可能成瓶颈；热局部变量反复写浪费带宽。
- **Write-behind**：CPU 不等写完继续跑，重叠延迟；若随后 miss，仍须等写与填完成。
- **Write-back**：只改 cache，换出时才写回主存——最少写主存，现代主流。

## 38. 写回（Write-Back）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/13a8e24c4ec050a58135258a720bd49d_Slide39.png" alt="Write-Back" width="80%"/>

写请求只更新 cache。替换时须先把旧行写回主存（若可能已被改过）——否则会写回从未写过的行，浪费带宽。

## 39. 带 Dirty 位的写回（Write-Back with Dirty Bits）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d16c64e90f2aa7ebe705742389b11463_Slide40.png" alt="Write-Back with Dirty Bits" width="80%"/>

每行加 **dirty** 位：填入时清 0；写命中置 1。仅 dirty=1 时换出才写回。CPU 只在 miss（且可能需写回脏行）时等待。

## 40. 小结：Cache 权衡（Summary: Cache Tradeoffs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d492e707ff27c9c46dc10d0d05702019_Slide41.png" alt="Summary Cache Tradeoffs" width="80%"/>

目标：层次存储 → 低 AMAT + 高容量。手段：更多行↓ miss ratio；合适块大小利用 DRAM 列突发；更多 way↓ 冲突；LRU（近似）选替换；write-back + dirty。最终用基准仿真在各项之间取最优组合。
