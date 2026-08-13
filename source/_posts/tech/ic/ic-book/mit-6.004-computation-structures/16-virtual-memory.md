---
title: MIT 6.004：L16 虚拟内存
date: 2026-08-11 10:16:00
categories: ic
tags:
  - ic
  - digital-circuit
  - memory
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L16 注解幻灯片。
>
> 源网页：[16.1 Annotated Slides | Virtual Memory](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c16/c16s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L16：虚拟内存（Virtual Memory）

本讲把存储层次从 cache/主存延伸到**辅存**（secondary storage）：用 **MMU** 做虚实地址翻译、按**页**（page）管理主存、用 **page fault** 按需装入，并用 **TLB** 加速页表访问；同时引入多程序 **context** 与保护的雏形。

## 1. 回顾：典型存储层次（Reminder: A Typical Memory Hierarchy）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ca68d32a905a29bfb21ade15fdb96eb9_Slide02.png" alt="Reminder A Typical Memory Hierarchy" width="80%"/>

回到《The Memory Hierarchy》里的基本权衡：容量越大，访问时间往往越长。要同时做到**大容量**与**小平均访问时间**，靠夹在 CPU 与主存之间的 **cache** 体系。现代 CPU 常有多级 cache：一级容量不大、接近 CPU 速度；更高级容量更大、延迟更长。

## 2. 回顾：硬件 Cache（Reminder: Hardware Caches）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/85258de6b1b2b4bd123d45aac81a9b78_Slide03.png" alt="Reminder Hardware Caches" width="80%"/>

Cache 对少量地址提供快速访问，用**相联寻址**装下 CPU 最近常用的位置；内容由硬件自动管理。有效性依赖**局部性**：访问 $X$ 后不久常访问邻近地址。组织上用简单索引选出候选行；引入**相联度**提高命中率，并讨论块大小、替换策略、写策略。本讲把层次再往下扩，会再次遇到同类设计选择。

## 3. 回顾：存储层次再往下（Reminder: A Typical Memory Hierarchy）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/21caeaaf53bdd4b74016c7fc5e0260e2_Slide04.png" alt="Reminder A Typical Memory Hierarchy continued" width="80%"/>

此前未谈主存数据从何而来。Flash / 硬盘等**辅存**容量更大且**非易失**：关机仍保留数据。开机时数据都在辅存；需要时再搬到主存（primary storage）。可把主存看成辅存之上的又一级 cache，并构建**虚拟内存**：按需自动从辅存装入主存，并控制程序可访问哪些数据——这是安全多道程序的垫脚石。

## 4. 扩展存储层次（Extending the Memory Hierarchy (continued)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6ff33cf0610d15b954e42727e0de1e85_Slide05.png" alt="Extending the Memory Hierarchy continued" width="80%"/>

在 L14 的 cache + 主存之上加上辅存。好处：容量极大（台式机 TB 级，云可达 PB，$1\,\mathrm{PB}=10^{15}$ 字节）。坏处：磁盘访问可比 DRAM 慢约 $10^5$ 倍——从 DRAM 到盘的跳变远大于 cache 到 DRAM。盘上连续块的边际代价更低，因此一次读较大块。主存 miss 的代价极高，虚拟内存必须把主存 miss 率压得**极低**（相对指令执行率）。

## 5. 巨大 Miss 惩罚的含义（Impact of Enormous Miss Penalty）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/78be758312fec925aabecb66e19d92b7_Slide06.png" alt="Impact of Enormous Miss Penalty" width="80%"/>

因此对“主存作辅存的 cache”要求：

- **高相联度**：工作集能放进主存时，应尽量避免无谓冲突
- **大块（页）**：摊薄盘访问固定开销，并利用局部性
- **write-back**：仅在脏页被替换时才写回辅存

miss 延迟极长带来一个好处：可用**软件**管理主存组织与盘 I/O——即便处理 miss 要执行数千条指令，仍远快于盘访问。策略：**命中用硬件，miss 用软件** → MMU 硬件可较简单，miss 处理可很聪明。

## 6. 虚拟内存（Virtual Memory）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f0d035769d245ce16469e97ebe908342_Slide07.png" alt="Virtual Memory" width="80%"/>

CPU 产生的地址称**虚地址**（virtual address），主存用**物理地址**。中间插入 **MMU**（memory management unit），用**页表 / page map** 把虚地址翻译成物理地址（本讲暂忽略 cache，末尾再谈二者并存）。

页表允许某虚地址映射到主存任意处；正常时两虚地址不宜映到同一物理地址；允许某些虚地址**无翻译**——表示尚未装入主存，MMU 发存储管理异常，由 CPU 分配物理页并从辅存装入。

页表还带来控制力：换程序时换页表即可**分时**；一程序可见的物理页可对另一程序不可见；可用异常做**按需装入**，只需保证**工作集**在主存。

## 7. 实现：分页（Virtual Memory Implementation: Paging）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/494f65fbf41a6f5319cf0acc09b26f77_Slide08.png" alt="Virtual Memory Implementation Paging" width="80%"/>

逐地址映射表过大，故把虚、实地址空间都切成固定大小的**页**，大小 $2^p$ 字节。低 $p$ 位为**页内偏移**（page offset），其余为**页号**。典型 $p=12\sim 14$（4KB～16KB）。

例：32 位虚地址、$p=12$ → 高 20 位 **VPN**（virtual page number），低 12 位偏移。物理地址同理拆成 **PPN** + 偏移。MMU 按页管理：整页从辅存搬入主存。偏移取自低位，使邻近数据多在同一页。

翻译：用 VPN 索引页表；表项指示是否在主存，若在则给出 PPN，再与偏移拼成物理地址。若不在 → **page fault**，由 OS 装页并更新映射。主存作页 cache 的方案称 **paging / demand paging**。

## 8. 按需分页（Demand Paging）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/92ff634a39a20e72fefa3c41ef726849_Slide09.png" alt="Demand Paging" width="80%"/>

初始：程序各虚页在辅存，MMU 无驻留映射。CPU 每次访存经 MMU；命中则主存完成访问；不命中 → page fault → **page fault handler**：分配物理页、从辅存装入、更新页表。

若无空闲物理页，选一驻留页替换（如近期未用）：脏页先写回，再标为不驻留，腾出物理页。工作集经一串 fault 装入后，若程序行为良好，fault 频率可接近零；不断 fault 称 **thrashing**，因辅存极慢，程序会“爬行”。

## 9. 简单页表设计（Simple Page Map Design）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0139cfa5e58addab62f0c4a4ac8808a7_Slide10.png" alt="Simple Page Map Design" width="80%"/>

每个虚页一条表项。例：32 位虚地址、$2^{12}$ 字节页 → VPN 20 位 → $2^{20}$ 条表项。

每项至少含：

- **R（resident）**：1 表示在主存；0 则访问触发 page fault
- **PPN**：R=1 时给出物理页号
- **D（dirty）**：刚从辅存装入时 clean（D=0）；CPU 写入后置 D=1；替换脏页须先写回

还可有只读位等：写只读页触发异常，利于保护代码页。

## 10. 例：虚→实翻译（Example: Virtual → Physical Translation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8921d7b1ee1049674797f98ea405cd89_Slide11.png" alt="Example Virtual to Physical Translation" width="80%"/>

简化例：虚地址 12 位 = 4 位 VPN + 8 位偏移（16 个虚页）；物理地址 11 位 = 3 位 PPN + 8 位偏移（8 个物理页）。页表 16 项 ×（D+R+3 位 PPN）= 80 比特。物理页上虚页号可任意打乱——取决于 fault 时哪页空闲。

例：`LD` 访问虚地址 `0x2C8` → VPN=2，偏移=`0xC8`。表项 2：R=1，PPN=4 → 物理地址 `0x4C8`。**偏移在翻译中不变**。

## 11. Page Fault（Page Faults）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bffeaec8388feaf5d6bebe7b326896ca_Slide12.png" alt="Page Faults" width="80%"/>

访问 R=0 的虚页 → page fault → 挂起程序，进入 handler。找空闲物理页，或选一在用页腾出：若 D=1 则写回，再把被替虚页标为不驻留。

限制：不能换出 handler 自身所在页（**wired**）；也不宜换出即将继续执行的代码页。理想是换“最远将来才再用”的页，但需未来信息；实践中有多种替换算法（如 aging，近似最优且实现代价适中）。

然后把目标虚页读入选定物理页，更新其 R/PPN，再**重执行**触发 fault 的指令——此时映射已就绪，访问成功。

## 12. 例：Page Fault（Example: Page Fault）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/94b7e209c14448734b27427bb9ad0ceb_Slide13.png" alt="Example Page Fault" width="80%"/>

同一设定下，`ST` 访问虚地址 `0x600`（VPN 6）。表项 R=0 → fault。设选 LRU 页 VPN `0xE` 替换：其 D=1，故写回 PPN `0x5` 内容，再标 `0xE` 不驻留。从辅存把 VPN 6 装入 PPN `0x5`，更新表项。恢复执行并重做 `ST`：`0x600` → 物理 `0x500`，且因写入将 D 置 1。

## 13. CS 视角（Virtual Memory: the CS View）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/849c7053bd6513abbd4299daaa479a00_Slide14.png" alt="Virtual Memory the CS View" width="80%"/>

把 MMU 工作看成两个过程。页表信息可视为数组：R[]、D[]、PPN[]、DiskAdr[]。

- **VtoP**：每次访存调用；若虚页不驻留则调 **PageFault**；再取 PPN，与偏移拼接得物理地址
- **PageFault**：选替换页、脏则写回、标不驻留、从辅存读入目标页并更新映射

## 14. 硬件 / 软件分工（The HW/SW Balance）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5cd8378a9e6d275837313597c27210a9_Slide15.png" alt="The HW SW Balance" width="80%"/>

**VtoP 用硬件**（每次访存都要）；**PageFault 用异常进软件**。通则：快路径硬件，罕发异常软件。所谓“软件”仍跑在 CPU 上，实质是专用硬件（MMU）与通用硬件（CPU）的权衡——应对“真正常见且性能关键”的操作才上专用硬件。

## 15. 页表参数算术（Page Map Arithmetic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4ffe8190f764756b76b3e07bb79b97f8_Slide16.png" alt="Page Map Arithmetic" width="80%"/>

三个架构参数：$p$（页偏移位数）、$v$（VPN 位数）、$m$（PPN 位数）；其余由此导出。页大小常在 4KB～16KB：太大浪费装入无用字，太小摊不薄盘开销。

虚地址宽度由 ISA 定：从 32 位（4GB）迈向 64 位（约 $2^{64}$ 字节，$\mathrm{exa}=10^{18}$）。虚地址过小曾导致许多 ISA 消亡。物理地址宽度可随实现代际调整（嵌入式约 30 位，服务器 40+ 位）；程序员用虚地址，由 MMU 屏蔽物理容量差异——功能不变，性能可变。

## 16. 算术例（Example: Page Map Arithmetic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3c30be5daf2b6cffff8d53aa46e06a80_Slide17.png" alt="Example Page Map Arithmetic" width="80%"/>

设虚 32 位、物理 30 位、页 4KB：$p=12$，$v=20$，$m=18$。物理页数 $2^{18}$，虚页数 $2^{20}$，页表项数约 $10^6$。每项约 $m+2=20$ 比特 → 页表约 20 Mbit。若用专用大 SRAM 存整表会很贵。

## 17. 页表放主存（RAM-Resident Page Maps）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/735aed71c9919931b0705ea749b6927f_Slide18.png" alt="RAM-Resident Page Maps" width="80%"/>

何必专用存储器？用**页表指针寄存器**指向主存中的页表数组，页表占若干物理页。用 VPN 做数组下标取表项即可。代价：一次虚访问需**两次**物理访问——先读页表项，再访问目标。

## 18. TLB（Translation Look-aside Buffer (TLB)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/262260a2204fd14bbc83bce39b724676_Slide19.png" alt="Translation Look-aside Buffer TLB" width="80%"/>

引入专用小而快的 cache——**TLB**，缓存 VPN→PPN。常全相联以提高命中、避免冲突。TLB 命中则可省掉读页表，虚访问回到一次物理访问。命中率常 $>99\%$：短期工作集页数不多。基本策略不变，细节可有多种变体。

## 19. MMU 地址翻译流程（MMU Address Translation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c576baea530ba0b9d74850a6183e8255_Slide20.png" alt="MMU Address Translation" width="80%"/>

流程：先查 TLB；命中则直接访主存。未命中则读页表：若页驻留，用 PPN 完成翻译并**填入 TLB**；若不驻留 → page fault，交 handler。

## 20. 综合例：带 TLB 的 MMU（Putting it All Together: MMU with TLB）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c36914583c40a48b48a465121938bdd6_Slide21.png" alt="Putting it All Together MMU with TLB" width="80%"/>

例：$p=10$，$v=22$，$m=14$。

- 物理页数 $2^{m}=2^{14}$
- 页表项数 $2^{v}=2^{22}$
- 每项 $m+2=16$ 比特
- 页表总字节约 $2^{23}$，占 $2^{13}$ 页
- 同时可驻留比例 $2^{m}/2^{v}=1/2^{8}$

翻译例：虚 `0x1804` → 偏移 `0x004`，VPN `0x6`，TLB 命中 PPN `0x2` → 物理 `0x804`。虚 `0x1080`：TLB 未命中，页表给出 PPN 5 → `0x1480`。虚 `0x0FC`：TLB/页表均显示不驻留 → page fault。注意：替换虚页时把页表 R 置 0 后，也须使对应 **TLB 项失效**。

## 21. Context（Contexts）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a9a925579c0724ab7aa796c8e2df9edb_Slide22.png" alt="Contexts" width="80%"/>

页表提供解释虚地址的 **context**：同一虚地址 0 在不同程序映到不同物理位置。多程序可各有独立虚地址空间并共享物理内存。换程序即换 context（重载页表相关状态）。

## 22. Context 预告：分时与 OS（Contexts: A Sneak Preview）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1236159c243160424f6522fcc3f269a4_Slide23.png" alt="Contexts A Sneak Preview" width="80%"/>

分时系统周期性地在程序间切换 CPU，造成“各有一台虚拟机”的错觉——切换时同时切换 CPU 状态与 MMU context。特权代码 **OS** 运行在 **kernel** context：管理物理内存与异常；用户程序在 **user mode**。异常进入 kernel mode；处理完再回 user mode。内核可访问 MMU、I/O 等特权寄存器；用户要通过 OS 请求盘等服务。下讲展开。

## 23. 内存管理与保护（Memory Management & Protection）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/331fa99f4674afcec9baf7e04c1baa38_Slide24.png" alt="Memory Management and Protection" width="80%"/>

用户程序仿佛独占整个虚地址空间，常遵守相同约定（入口、栈初值等）；OS 靠不同 context 隔离。典型布局：虚页 0 不可访问（抓空指针）；接着只读代码（及共享库）；再读写静态数据；其余由向高地址增长的**栈**与向低地址增长的**堆**对向扩张。区域增长时 fault handler 可分配新页；若中部相遇则虚存耗尽。

## 24. 多级页表（Multi-level Page Maps）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d0b3392a0836792eddc64fcd4e03d992_Slide25.png" alt="Multi-level Page Maps" width="80%"/>

扁平页表占大量物理页；多 context 时更糟。**分层页表**：虚地址高若干位索引 **page directory**，得到该段页表所在物理页；页表段本身也可在虚存中、不必全部驻留。栈与堆之间未分配区在 directory 中标不驻留，无需为“全不驻留”的海量表项占空间。代价是 walk 多一次访存，但 **TLB** 使额外开销通常可忽略。

## 25. 快速 Context 切换（Rapid Context Switching）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7da47edef444ec4a3b371119002ec739_Slide26.png" alt="Rapid Context Switching" width="80%"/>

换 context 时重载页表指针相当于换掉整张表，常需**冲刷 TLB**，随后命中率骤降。改进：引入 **context-number** 寄存器，与 VPN 一起作 TLB 查询（tag 含 context）。切换时重载 context-number 与页表指针即可；其他 context 的 TLB 项自然不匹配，**无需 flush**。TLB 容量够时，多 context 映射可并存，切换对平均访存时间冲击小。

## 26. Cache 与虚拟内存（Using Caches with Virtual Memory）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d9eb4240d1113e980d33fcd5082ad4af_Slide27.png" alt="Using Caches with Virtual Memory" width="80%"/>

- **虚地址 cache**（CPU 与 MMU 之间）：仅 miss 时付翻译代价；但 context 切换改变虚存含义，常需冲刷 cache，切换代价大
- **物理地址 cache**（MMU 与主存之间）：切换不使 cache 语义失效；但须先翻译再查 cache，略增平均延迟

## 27. 并行：两全其美（Best of Both Worlds: Overlapped Operation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0842d364b1a1a934db837156df6da3de_Slide28.png" alt="Best of Both Worlds Overlapped Operation" width="80%"/>

若 cache 的 **line index** 完全落在页偏移内，则这些位不受 MMU 影响，可与 TLB/翻译**并行**启动查 cache；再用物理地址 tag 做比较。TLB 命中时，物理地址与 cache tag 大致同时就绪 → **物理寻址 cache，几乎无翻译惩罚**。

推论：增大 cache 容量时，若要保持 index⊆偏移，不能单靠增加 line 数或块大小（会吃掉偏移位），往往靠提高**相联度**。

## 28. 小结（Summary: Virtual Memory）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a937426b9ad39c2b6552f345a1f2c8c6_Slide29.png" alt="Summary Virtual Memory" width="80%"/>

MMU 提供虚→实映射的 context；切换 context 可造出多个虚地址空间，多程序共享 CPU 与物理内存而不互扰。页表做 VPN→PPN；页表放主存，用 **TLB** 省掉多数页表访问；不驻留页触发 **page fault**，由 OS 公平管理物理页。Context 是迈向**虚拟机 / 处理器虚拟化**的第一步——即下一讲主题。
