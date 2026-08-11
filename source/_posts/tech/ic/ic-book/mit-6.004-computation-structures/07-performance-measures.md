---
title: MIT 6.004：L07 性能度量
date: 2026-08-11 17:22:06
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L07 注解幻灯片。
>
> 源网页：[7.1 Annotated Slides | Performance Measures](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c7/c7s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L07：性能度量（Performance Measures）

本讲引入电路性能的两个核心指标——**延迟**（latency）与**吞吐**（throughput），并用洗衣店类比说明流水线如何用略增延迟换取更高吞吐；随后给出构造良构 **K 级流水线**（K-pipeline）的系统方法，以及用**交错**（interleaving）与**并行**突破瓶颈；最后对比同步全局定时、握手与自定时等控制结构。

## 1. 先放下电路… 解决一个真实问题（Forget Circuits… Let’s Solve a Real Problem）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d6582155063b1c16b910994c85863d39_Slide02.png" alt="Forget Circuits Let's Solve a Real Problem" width="80%"/>

目标：提出衡量电路性能的指标，并研究如何提升。先放下电路，用日常例子帮助理解这些指标。

洗衣是人人都会遇到的“处理任务”。系统输入是若干篮脏衣，输出是洗净、烘干并叠好的同批衣物。两个部件：洗衣机 30 分钟洗一篮，烘干机 60 分钟烘一篮。路径简单：先洗后烘；假设尽快推进，一步能走就走下一步。攒多篮再洗往往更划算——下面说明原因。

## 2. 一次一篮（One Load at a Time）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e811412181958549fbd020ffc7112bdd_Slide03.png" alt="One Load at a Time" width="80%"/>

单篮：洗 30 分钟 + 烘 60 分钟 = 从输入到输出共 90 分钟。若比作组合逻辑，传播延迟就是 90 分钟。接着考虑 $N$ 篮。

## 3. 洗 N 篮：哈佛式（Doing N Loads of Laundry）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/87a4ef9b1ec887d44fdd397ec4b8d8d4_Slide04.png" alt="Doing N Loads of Laundry Harvard way" width="80%"/>

想象“哈佛式”做法：严格按组合逻辑节奏——上一组输入算完、输出有效后，才喂下一组。步骤 1 洗第一篮，步骤 2 烘第一篮，共 90 分钟；再进入步骤 3 处理第二篮……总时间 $N\times 90$ 分钟。

类比清晰：烘干时洗衣机空闲，吞吐受拖累。

## 4. 洗 N 篮：6.004 式（Doing N Loads… The 6.004 Way）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/18fb620ed12f44707f03bbc0c2e86907_Slide05.png" alt="Doing N Loads the 6.004 Way" width="80%"/>

重叠洗与烘：步骤 1 洗第一篮；步骤 2 烘第一篮的同时开始洗第二篮。步骤时长由烘干机决定为 60 分钟；洗衣机虽提早完成，但只有一台烘干机，系统节拍由最慢级决定。

这种重叠处理输入序列的系统叫**流水线**（pipelined system），每步叫一级（stage）。洗衣是 2 级流水线，每级处理时间 60 分钟。稳态下每 60 分钟开一篮洗、出一篮干——有效吞吐为每 60 分钟一篮；$N$ 篮约 $N\times 60$ 分钟。某一篮要经两级，约 120 分钟（首篮略不同；流水线性能分析关心有无穷输入的稳态）。

## 5. 性能度量（Performance Measures）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/304cac088dc2ec2d71b1dea906bc6cff_Slide06.png" alt="Performance Measures" width="80%"/>

两个指标：

| 指标 | 含义 | 哈佛洗衣 | 6.004 洗衣 |
|------|------|----------|------------|
| **延迟**（latency） | 某一输入从进到出的时间 | 90 min | 120 min（非首篮） |
| **吞吐**（throughput） | 系统产生输出的速率 | 每 90 min 一篮 | 每 60 min 一篮 |

一入一出系统中，吞吐也等于输入被消耗的速率。哪个更好取决于目标：洗 100 篮要高吞吐；约会前 90 分钟内要干净内衣则更关心延迟。用流水线提吞吐时，各级需锁步，节拍由最慢级决定，延迟通常上升——这是常见的延迟–吞吐权衡。

## 6. 回到电路（Okay, Back to Circuits…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1865d06fbffabb7ed55d3495e6e3fc49_Slide07.png" alt="Okay Back to Circuits" width="80%"/>

组合电路：延迟 $= t_{\mathrm{PD}}$，吞吐 $= 1/t_{\mathrm{PD}}$（算完当前才开下一组）。

例：F、G 并行，结果再进 H。输入 $X$ 稳定后，F、G 出 $F(X)$、$G(X)$，再经 H 的 $t_{\mathrm{PD}}$ 得 $P(X)$。模块不变则延迟难再压；但 F、G 出结果后空闲等 H——能否让 F、G 处理下一输入，同时 H 仍在算上一拍？即把电路拆成两级：第一级算 $F(X)$、$G(X)$，第二级算 $H$。

## 7. 流水线电路（Pipelined Circuits）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3c49942983b8923bf8468502c767cb90_Slide08.png" alt="Pipelined Circuits" width="80%"/>

用寄存器锁住 $F(X)$、$G(X)$ 供 H 使用，同时让 F、G 开下一输入。分析时先假定流水寄存器 $t_{\mathrm{PD}}$、建立时间为 0。

时钟周期由最慢级决定：含 F、G 的级至少需 20 ns；含 H 的级需 25 ns → 系统 $t_{\mathrm{CLK}}=25\,\mathrm{ns}$。一般做法：寄存器把组合逻辑切成若干级，每拍推进一级。本例 2 级、周期 25 ns → 延迟 50 ns（略高于未流水），吞吐每 25 ns 一个输出——吞吐明显提升，延迟略增。

## 8. 流水线时序图（Pipeline Diagrams）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6ea4cc61191c176d87723be128e03557_Slide09.png" alt="Pipeline Diagrams" width="80%"/>

行表示各级，列表示连续时钟周期。周期 $i$ 初输入 $X_i$ 有效；该周期内 F、G 处理，$i$ 末结果写入级间寄存器。周期 $i+1$ 中 H 处理 $X_i$，同时 F、G 处理 $X_{i+1}$。某一输入沿对角线每拍下移一级；$i+1$ 末 H 结果写入末级寄存器，于 $i+2$ 可用。总延迟两拍；之后每拍出一个结果。

## 9. 流水线约定（Pipeline Conventions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3a00e78ac07876065860e2558dca63e2_Slide10.png" alt="Pipeline Conventions" width="80%"/>

**K 级流水线**（K-pipeline）：无环电路，从输入到输出**每条路径恰有 K 个寄存器**。未流水的组合电路是 0 级流水线。

约定：每一级（因而每个 K-pipeline）在**输出端**有寄存器，便于把流水部件拼成更大流水系统。用时序电路计时方法：对每条寄存器→寄存器、输入→寄存器路径，求输入寄存器 $t_{\mathrm{PD}}$ + 组合 $t_{\mathrm{PD}}$ + 输出寄存器建立时间，取最大者作为 $t_{\mathrm{CLK}}$ 下界。时钟正确且每条路径恰 K 个寄存器时，K-pipeline 与原组合电路功能等价。

- 延迟：$K\cdot t_{\mathrm{CLK}}$
- 吞吐：$1/t_{\mathrm{CLK}}$

## 10. 病态流水线（Ill-Formed Pipelines）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d4554aabe86a12a8bf6916fd9aa4343e_Slide11.png" alt="Ill-Formed Pipelines" width="80%"/>

反例：经 A、C 的上路径 2 个寄存器，经 B、C 的下路径 2 个，经 A、B、C 的中路径只有 1 个——不是良构 K-pipeline。后果：不同“代”的输入混算（如周期 $i+1$ 中 B 用当前 $X$ 与上一拍 $Y$），结果与未流水电路不同。需要一种方法：在一条路径加寄存器时，保证所有路径同步加。

## 11. 流水线方法论（A Pipelining Methodology）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1c6066766cac53e2989193aea6d86ad8_Slide12.png" alt="A Pipelining Methodology" width="80%"/>

步骤：

1. 画一条穿过所有**输出**的等高线，两端记为后续等高线的端点。
2. 继续在模块间信号上画等高线，两端仍落在同一对端点；每条信号同向穿越等高线（输入一侧、输出另一侧）。等高线划分流水级。
3. 在信号与等高线交点放置流水寄存器。

端点到端点保证每条输入–输出路径都被穿过 → 良构。再找最长寄存器→寄存器或输入→寄存器延迟定 $t_{\mathrm{CLK}}$。例：理想零延迟寄存器时，C 模块要求 8 ns → 吞吐每 8 ns 一个；3 条等高线 → 3-pipeline，延迟 $3\times 8=24\,\mathrm{ns}$。

通常目标：用尽量少的寄存器达到最大吞吐——找出最慢部件（如 C），在其输入输出两侧画等高线，把 $t_{\mathrm{CLK}}$ 钉在该延迟，再布置其余等高线使级间最长路径不超过此时钟。同吞吐/延迟下画法可有多种（如 E 可与 F 同级）。

## 12. 流水线示例（Pipeline Example）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f3bac46be064881fff7c90377f866a7d_Slide13.png" alt="Pipeline Example" width="80%"/>

复习：先过全部输出 → 1-pipeline，吞吐/延迟与原组合相同。再画线隔离最慢件 → 2-pipeline，$t_{\mathrm{CLK}}=2$，吞吐翻倍。再加等高线只会加级数、增延迟，不提吞吐（合法但不划算）。路径长短不一时会出现背靠背寄存器，正常。

策略：流水通常提吞吐、增延迟；各级完美平衡时延迟可不增。流水电路的延迟**不会**低于未流水电路。隔离最慢件后吞吐无法再靠加级提升——瓶颈在该部件本身。

## 13. 流水线部件（Pipelined Components）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a03876c64c760542b04daac6d68d952c_Slide14.png" alt="Pipelined Components" width="80%"/>

若有流水版部件可用：把 A 换成 2 级流水的 A′，重画等高线时须让 **2 条**等高线穿过 A′（计入内部寄存器），从而在系统其余处补上对齐两拍延迟的寄存器。例中各级最大 $t_{\mathrm{PD}}$ 降到 1 ns，吞吐从 $1/2$ 升到 $1/1$；4-pipeline，延迟 4 ns。若瓶颈无流水替代品，见下一节。

## 14. 6.004 学生怎么洗衣？（How Do 6.004 Students Do Laundry?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/51a7b346cc008492b2fe858bbc87aa2d_Slide15.png" alt="How Do 6.004 Students Do Laundry" width="80%"/>

绕过烘干机瓶颈：找“一台洗衣机配两台烘干机”的店。时间轴按 30 分钟一步：洗衣机每步都在用，每 30 分钟出一篮湿衣；烘干机交错——#1 烘奇数篮、#2 烘偶数篮，各烘两步（60 分钟）。整体每 30 分钟出一篮干净干衣。稳态吞吐每 30 分钟一篮，单篮延迟仍 90 分钟。

要点：烘干机本身未流水，但两台交错表现得像周期 30 分钟、延迟 60 分钟的 2 级流水线——用两份未流水部件交错，可等效 2 级流水。

## 15. 回到瓶颈（Back to Our Bottleneck…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0ff3be50c93f8a0bdae7377a59c50003_Slide16.png" alt="Back to Our Bottleneck" width="80%"/>

前例吞吐卡在 $1/8\,\mathrm{ns}$（C 的 8 ns 延迟定最小时钟）。要再提吞吐：要么找 C 的流水版，要么用两份未流水 C 做交错等效 2 级流水。

## 16. 电路交错 I（Circuit Interleaving I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a9bbd6a9fb3ac12430d195d6ab2acb52_Slide17.png" alt="Circuit Interleaving I" width="80%"/>

通用双路交错：两份 C（$C_0$、$C_1$）。各输入来自 D 锁存器（捕获并保持输入）；MUX 选哪路 C 输出写入输出寄存器。左下角极简 2 状态 FSM：下一状态为单反相器，状态在 0/1 间交替；状态位在上升沿后翻转。

## 17. 电路交错 II（Circuit Interleaving II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ecff151361dc49a6385d43586a4206be_Slide18.png" alt="Circuit Interleaving II" width="80%"/>

波形：新 $X$ 在时钟上升沿后到达。FSM $Q$ 为低时 $C_0$ 输入锁存打开，$X_1$ 进入，$C_0$ 在约第 2 拍末出结果；第 2 拍初锁存关闭，保持 $X_1$ 稳定，使 $C_0$ 有近两拍时间计算。$C_1$ 类似，错开一拍。MUX：$Q$ 高选 $C_0$，低选 $C_1$；上升沿时输出寄存器采样。行为像 2 级流水：周期 $i$ 到达的输入经两拍，于 $i+2$ 可用。

## 18. 电路交错 III（Circuit Interleaving III）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2268c55549658764fca3a121f8f01ae5_Slide19.png" alt="Circuit Interleaving III" width="80%"/>

时钟周期：上游流水寄存器、内部锁存与 MUX 的传播延迟，以及输出寄存器建立时间，使 $t_{\mathrm{CLK}}$ 须略大于 C 的 $t_{\mathrm{PD}}/2$。可把交错电路当作 2 级流水：每拍吃一个输入，两拍后出结果。

## 19. 组合技巧（Combine Techniques）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c6174e7cedd15d7a8e92944c4f1e39db_Slide20.png" alt="Combine Techniques" width="80%"/>

在流水图中，N 路交错部件当作 N 级流水，须有 N 条等高线穿过它。用 2 路交错的 C′ 替换慢 C：先过全部输出，再保证两条等高线穿过 C′，在交点加寄存器——穿过 C′ 的等高线会在 F 的其他输入上多加寄存器，以匹配 C′ 的两拍延迟。乐观设 C′ 最小 $t_{\mathrm{CLK}}=4\,\mathrm{ns}$，则系统时钟由 F 的 5 ns 决定 → 吞吐每 5 ns 一个；5 条等高线 → 5-pipeline，延迟 $5\times 5=25\,\mathrm{ns}$。

## 20. 再加一点并行（And Add a Little Parallelism…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/de7bb56a3ba1632bf184be09d64917e5_Slide21.png" alt="And Add a Little Parallelism" width="80%"/>

流水系统再并行：两台洗衣机 + 四台烘干机 ≈ 两份“1 洗 2 烘”。每步消耗/产出两篮 → 吞吐 2 篮 / 30 min，等效每 15 分钟一篮；单篮延迟仍 90 分钟。

即便部件慢，也可靠交错与并行继续提吞吐。上界存在：流水寄存器与交错逻辑的时序开销给出最小可行 $t_{\mathrm{CLK}}$，从而限制最大吞吐——现实世界没有无限加速。

## 21. 控制结构备选（Control Structure Alternatives）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ea6e83a9fb0166ece1d8f4be16600645_Slide22.png" alt="Control Structure Alternatives" width="80%"/>

此前流水各级锁步，时钟按各级最坏情况选取——**同步、全局定时**系统。

若处理时间有数据相关性（某些输入某级可更快）？一种做法：仍用单一系统时钟，但各级用握手表明“准备好要新输入 / 有新输出给下级”。两信号协议：上游 `HERE-IS-X` 表示下一上升沿将有新数据；下游 `GOT-X` 表示愿意在上升沿取数。仅在上升沿检查信号：两边同时看到两者为真则握手完成、该沿完成传输；任一方仍在忙可推迟。

也可做无全局时钟的异步自定时：四相握手——(1) 上游有新输出且 `GOT-X` 已撤时断言 `HERE-IS-X`；(2) 下游消费后断言 `GOT-X`；(3) 下游等 `HERE-IS-X` 变低；(4) 下游撤 `GOT-X`，准备下一轮。时序由握手边沿决定，无需全局时钟。

## 22. 自定时示例（Self-Timed Example）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/386f6b54bc5a9d726f41408893285d29_Slide23.png" alt="Self-Timed Example" width="80%"/>

多下游时：A 的输出同时给 B、C。黄色组合块合并 B、C 的 `GOT-X`，汇总给 A——两边都断言 `GOT-X` 后才向 A 断言；两边都撤消后才向 A 撤消。

## 23. 自定时示例（续）（Self-Timed Example continued）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3fc94c91527edd265a92dd065d445713_Slide24.png" alt="Self-Timed Example continued" width="80%"/>

动画式时序：上游向 A 传输入 → A 回 `GOT-X` → 完成四相。A 向 B、C 发 `HERE-IS-X`；B 先就绪断言 `GOT-X`，C 尚等第二输入。B 算完再给 C 第二输入；C 两边都收到后断言，黄盒才让 A 知道传输完成。整系统纯靠翻转信令：各模块自主决定消费/产出时刻，可按实际速度跑，而不必等全局最坏延迟。

## 24. 控制结构分类（Control Structure Taxonomy）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a993459f2f9be5250c436968bbc8c192_Slide25.png" alt="Control Structure Taxonomy" width="80%"/>

| 方式 | 特点 |
|------|------|
| **同步全局定时** | 时钟按最坏情况；易设计；无法利用数据相关加速 |
| **同步 + 握手** | 仍在时钟上升沿通信；具体哪一拍传输由级间握手决定 |
| **调节全局时钟** | 理论上可跟数据相关提速；大系统定时器极复杂，通常不如局部通信 |
| **局部定时异步** | 每代工程师都会被吸引；大系统（如现代 CPU）难做出可证明可靠的设计；特殊场景（如整数除法）数据相关加速值得额外投入 |

## 25. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0f5d1622c5859b68fac7a42454648c48_Slide26.png" alt="Summary" width="80%"/>

- 用**延迟**与**吞吐**刻画性能；组合电路延迟 $= t_{\mathrm{PD}}$，吞吐 $= 1/\text{latency}$。
- **K-pipeline**：每级输出有寄存器；输入到输出每条路径恰 K 个寄存器；$t_{\mathrm{CLK}}$ 由最慢级决定；吞吐 $= 1/t_{\mathrm{CLK}}$，延迟 $= K\, t_{\mathrm{CLK}}$。
- 流水线是多数高性能数字系统提升吞吐的关键；交错与并行可在慢部件存在时继续推高吞吐，但受寄存器/交错开销限制。
