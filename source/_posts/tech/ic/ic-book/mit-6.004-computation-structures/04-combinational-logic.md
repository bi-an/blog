---
title: MIT 6.004：L04 组合逻辑
date: 2026-08-11 10:04:00
categories: ic
tags:
  - ic
  - digital-circuit
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L04 注解幻灯片。
>
> 源网页：[4.1 Annotated Slides | Combinational Logic](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c4/c4s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L04：组合逻辑（Combinational Logic）

本讲介绍如何把功能规格落地为组合逻辑电路：真值表与布尔方程、积之和（sum-of-products）综合、宽门与反相逻辑、布尔化简与卡诺图、MUX / ROM 等实现策略。

## 1. 功能规格（Functional Specifications）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e8df789c8bc9acecf023596506c4f7c7_Slide02.png" alt="Functional Specifications" width="80%"/>

功能规格是构建组合逻辑抽象时**静态纪律**（static discipline）的一部分。可用自然语言描述器件行为：表达紧凑、人人能读，但措辞不严时易有歧义，也难判断是否穷尽所有情况。

更好的替代：

- **真值表**（truth table）：对每种数字输入组合明确给出输出。$N$ 个数字输入 → $2^N$ 行。例：3 输入共 $2^3=8$ 行，可系统枚举，不易漏项，输出显式，歧义少。输入很多时不实用——两个 32 位加数共 64 输入，真值表需 $2^{64}$ 行；若每秒填一行，约需 5840 亿年。
- **布尔方程**（Boolean equation）：用 AND、OR、XOR（二元）与 NOT（一元）由输入算输出。0 ↔ FALSE，1 ↔ TRUE。

记号：输入用名字（如 $A,B,C$）；NOT 用上划线（如 $\overline{C}$）；AND 用乘法（显式点或隐式并置）；OR 用 $+$。输入多时布尔方程更合适，也易于画成电路图。真值表与布尔方程可互化：代入输入求方程可填表；由真值表可写成**积之和**（sum-of-products, SOP）。

## 2. 一种设计路径（Here's a Design Approach）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5f2cd2568ea6f9671bec2091457e5347_Slide03.png" alt="Sum-of-products from truth table" width="80%"/>

问：“何时 $Y=1$？”——即何时 $Y$ 为 TRUE。若第 2、4、7、8 行 $Y=1$，则方程是这四项之 OR；每一项是对应该行输入组合的积项。

例：第 2 行 $C=0,B=0,A=1$ → $\overline{C}\cdot\overline{B}\cdot A$。第 4 行 → $\overline{C}\cdot B\cdot A$，其余类似。所得恒为积之和：“和”指 OR，“积”指 AND 组。

## 3. 积之和的积木（Sum-of-products Building Blocks）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c07c36559eacf00cf12ec0f508f43c85_Slide04.png" alt="Sum-of-products Building Blocks" width="80%"/>

电路库由厂商提供，或自己用 NFET/PFET 做成 CMOS 门：

- **反相器**（inverter）：输出小圆表示反相；实现 NOT
- **AND**：全为 1 才输出 1；库中常有 3、4 输入等
- **OR**：至少一个输入为 1 则输出 1；同样有多输入版本

原理图：AND 输入侧平直，OR 输入侧弯曲。

## 4. 直接综合（Straightforward Synthesis）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8af7e08bd8a5bb341fde8a6b4bdae59e_Slide05.png" alt="Straightforward Synthesis" width="80%"/>

电路结构追随方程：反相器做 NOT（SOP 中常对输入取反；原理图可为每个 NOT 单独画反相器，实作可共享 $\overline{C}$ 等信号）；每个积项用多输入 AND；再用多输入 OR 合并。典型层次：反相器层 → AND 层 → OR。

传播延迟 $t_{\mathrm{PD}}$ 看似短（最长路径约反相器 + AND + OR），但宽 AND/OR 往往要多层门拼出，会增加延迟。至此已有：真值表 → SOP 方程 → 门电路。

## 5. 多于 2 输入的 AND / OR（ANDs and ORs with > 2 Inputs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/98309f2f26ff4606dbff12c25850ced8_Slide06.png" alt="Wide AND and OR from 2-input gates" width="80%"/>

假设库只有 2 输入门。AND 满足结合律，可用两两 AND 任意顺序做 $N$ 路 AND；OR、XOR 同理。

- **链**（chain）：$N$ 输入需 $N-1$ 个 2 输入门；$t_{\mathrm{PD}}$ 约 $N-1$ 级，随 $N$ 线性增长
- **树**（tree）：先并行再合并；$t_{\mathrm{PD}}$ 约 $\log_2 N$ 级，$N$ 大时明显更快

成本（门数）两者相同。若各输入到达时间不同（如前级 $t_{\mathrm{PD}}$ 不同），晚到输入经树可能还要多级，经链可能只多一级——**子电路哪种最优，取决于输入何时到达**。

CMOS 中单级门天然反相，高性能更倾向 NAND/NOR（单级 CMOS）；AND/OR 常要两级（如 NAND+反相器）。NAND、NOR **不**结合：$\mathrm{NAND}(A,B,C)\neq\mathrm{NAND}(\mathrm{NAND}(A,B),C)$，不能简单用 2 输入 NAND 树拼宽 NAND。

## 6. 更多积木（More Building Blocks）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f8031bc12424d05f89d2867324ad286a_Slide07.png" alt="XOR and related building blocks" width="80%"/>

**异或**（XOR）在算术、奇偶校验中很有用；Lab 2 会看到 2 输入 XOR 比 2 输入 NAND/NOR 消耗更多 NFET/PFET。任意真值表都可写成 SOP，再用 INVERTER、AND、OR 实现。

## 7. 万能积木（Universal Building Blocks）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2478d8ad6d89b3eca5907f83fd10bcec_Slide08.png" alt="Universal Building Blocks" width="80%"/>

仅用 **2 输入 NAND** 即可实现任意功能——称其为**万能门**（universal gate）。SOP 的各积木都有 NAND-only 等价电路；**2 输入 NOR** 同样万能。反相逻辑初看别扭，却是 CMOS 低成本、高性能的关键。

## 8. CMOS 偏爱反相逻辑（CMOS Loves Inverting Logic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b8067bb5a97076d6616a07162f5245ee_Slide09.png" alt="CMOS Loves Inverting Logic" width="80%"/>

库中既有反相门（反相器、NAND、NOR），也有非反相门（缓冲器、AND、OR）。对比 4 输入 AND 的三种实现（数值以库文档为例，重在相对比较）：

| 实现 | 约 $t_{\mathrm{PD}}$ | 约面积 |
|------|----------------------|--------|
| 库中 4 输入 AND | 160 ps | 20 µm² |
| 4 输入 NAND + 反相器 | 90 ps | 稍大 |
| 2 输入门树（NAND + NOR） | 再省约 10 ps | 再大一点 |

非反相门常做成**小而慢**（MOSFET 更窄）；反相门做成**快**。整电路 $t_{\mathrm{PD}}$ 由**最长路径**决定：非关键路径可用更小更慢的门省面积。底层树电路还用到 **DeMorgan 定律**：$\overline{A+B}=\overline{A}\cdot\overline{B}$ 等，可把 NOR 看成带反相输入的 AND，使第一层反相输出与第二层反相输入“对消”，等价于 AND 树。

## 9. 宽 NAND / NOR（Wide NANDs and NORs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/aa3bcff582a56613312bdc9b534e927c_Slide10.png" alt="Wide NANDs and NORs" width="80%"/>

库中反相门通常做到约 4 输入：4 输入 NAND 的下拉链有 4 个串联 NFET，电阻累加；加宽管子又增大面积与输入电容。尺寸–速度权衡迅速变复杂，故库常止于 4 输入，更宽由设计者用 DeMorgan：交替 NAND/NOR 树拼 8 输入 NAND、NOR 等。中间层 NOR 可视为带反相输入的 AND，整体像带反相输出的 AND 树；中间层 NAND 则对应 OR 树。

## 10. CMOS 积之和实现（CMOS Sum-of-products Implementation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/da8ba482be5d19822a7911ee7d59b507_Slide11.png" alt="CMOS Sum-of-products Implementation" width="80%"/>

同一 SOP 可用两层 NAND 或两层 NOR 实现。对输出侧 NAND 用 DeMorgan 变成“带反相输入的 OR”，再与第一层反相输出对消气泡，即得：反相器层 + AND 层 + OR。NOR/NOR 同理。

NOR/NOR 往往更多反相器；但可能减轻输入负载（例：NAND/NAND 中 $A$ 接 4 个 MOSFET，NOR/NOR 中 $A$ 只经反相器接 2 个）。需要快的 AND/OR 型 SOP 时，优先试 **NAND/NAND**，通常比 AND/OR 明显更快。

## 11. 逻辑化简（Logic Simplification）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5e210be86ae6a606c1da441b528229b5_Slide12.png" alt="Logic Simplification" width="80%"/>

能否用更少/更小的门实现同一功能？布尔恒等式可变换表达式。**归约恒等式**（reduction identity）等可把含两变量、多次运算的式子压成更简单形式。例：四积项方程中，中间两项令 $\alpha=C\cdot B$，消去 $A$；再对另两项令 $\alpha=\overline{C}\cdot A$ 继续归约——运算次数可从约 14 降到约 4，电路更便宜、$t_{\mathrm{PD}}$ 更小。手工化简繁琐易错，实际多用程序；最优形式搜索随输入数超指数增长，大方程靠启发式，结果很好但不一定最优。

## 12. 布尔最小化（Boolean Minimization）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0d52b122c4f62b47a64ba7d1ad5a1f2b_Slide13.png" alt="Boolean Minimization" width="80%"/>

另一种思路：在真值表中找**无关**（don't-care）情形。例如原表第 1、3 行：$A=0$、$C=0$、$Y=0$，仅 $B$ 不同 → 当 $A,C$ 皆 0 时 $B$ 无关，在压缩表中把该行的 $B$ 记为 $X$。比较 $Y$ 相同的行，可继续找出其他 don't-care。

## 13. 带无关项的真值表（Truth Tables with Don't Cares）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0ff7dc667b8e1a06540a002aa6bb20ee_Slide14.png" alt="Truth Tables with Don't Cares" width="80%"/>

带 don't-care 的表中，$Y=1$ 的行往往更少。有的行冗余：例如某行匹配的输入组合（如 011 与 111）已被其他行覆盖。由第 2、4 行导出的积项，正是用归约恒等式得到的那些积项——几何/表格视角与代数归约殊途同归。

## 14. 为何不一定用最小 SOP（The Case for a Non-minimal SOP）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fa03492304960ed9f298478334c65da8_Slide15.png" alt="Glitches and non-minimal SOP" width="80%"/>

最小电路是否总最好？看 $A=1,B=1$、$C$ 从 1→0：原先由下 AND 维持 $Y=1$，过渡后上 AND 经反相器延迟才打开，中间可能两 AND 都关，$Y$ 短暂变 0——即 **毛刺**（glitch），传播会耗电。若保留第三积项 $BA$，则 $A,B$ 同高时 $Y$ 与 $C$ 无关，$C$ 翻转不引起 $Y$ 毛刺。上一章称此类电路为**宽容的**（lenient）。

## 15. 卡诺图：几何方法（Karnaugh Maps）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/860deb6d2c42017fc84931ce1fc8e221_Slide16.png" alt="Karnaugh Maps" width="80%"/>

最小化时要找可合并的相邻积项。**卡诺图**（Karnaugh map, K-map）把真值表排成二维：行/列用输入取值标记，格内为输出。列序用 **Gray 码**：相邻标签恰差一位；左右列也视为相邻（想成圆柱）。立方体上相邻的 3 位输入，在表中也相邻。

## 16. 扩展到 4 变量（Extending K-maps to 4-variable Tables）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3ff40a10e68588174ef469e02742f5cb_Slide17.png" alt="4-variable K-maps" width="80%"/>

4 变量：行、列皆 Gray 码；左右列、上下行相邻。6 变量需 $4\times4\times4$ 三维，难画；更多维更不现实。实践上 K-map 适合 ≤4 变量；思想可推广到更高维（计算机处理）。

## 17. 寻找蕴涵项（Finding Implicants）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e09fa19f4c140942792f5c48c82d7e49_Slide18.png" alt="Finding Implicants" width="80%"/>

**蕴涵项**（implicant）：K-map 中全为 1 的矩形区域；宽、高须为 2 的幂（1、2 或 4）。可重叠。**素蕴涵项**（prime implicant）：不被任何更大蕴涵项完全包含。最终最小 SOP 的每个积项对应某个素蕴涵项。

## 18. 寻找素蕴涵项（Finding Prime Implicants）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dfaf9e998252a038f68883a4eb60bac1_Slide19.png" alt="Finding Prime Implicants" width="80%"/>

例：孤立的 1 → $1\times1$；右上相邻两个 1 → $1\times2$。记得左右列相邻，可得到 $2\times2$ 等。对未圈住的 1，应找**包含它的最大**合法蕴涵项——更大素蕴涵项对应更短积项。

## 19. 写出方程（Write Down Equations）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b037e1c4b1bd3e354989562f0754f8f5_Slide20.png" alt="Write Down Equations" width="80%"/>

每个蕴涵项对应一个积项：只对在区域内保持不变的输入写文字。例：$A=0,B=0,C=1$ 的单格 → $\overline{A}\cdot\overline{B}\cdot C$；$1\times2$ 且 $C=0,A=1$ 不变 → $A\cdot\overline{C}$。素蕴涵项越大，积项越短。

做法：找未圈 1 → 圈包含它的最大合法矩形 → 重复，直至覆盖所有 1；再只保留覆盖所需的素蕴涵项，写出 SOP。最小 SOP **不必唯一**（不同素蕴涵项组合可等价），运算次数通常相同。≤4 变量时，K-map 比手搓恒等式更快、更不易错。

## 20. 素蕴涵项、毛刺与宽容性（Prime Implicants, Glitches & Leniency）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/21ed196dff6c639ef49d9005943ea17a_Slide21.png" alt="Prime Implicants Glitches Leniency" width="80%"/>

K-map 上也可见毛刺：从一蕴涵项跳到另一蕴涵项、中间有“缝”，输出可能毛刺。补上覆盖该过渡的素蕴涵项（哪怕功能上冗余），可堵住缝隙。**要宽容：把所有素蕴涵项都写进 SOP**。

## 21. 其实在设计多路选择器（We've Been Designing a MUX）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/980630cf696d1bb3050df1bc266ba5ab_Slide22.png" alt="Multiplexer" width="80%"/>

示例真值表描述的是 **2 选 1 多路选择器**（multiplexer, MUX）：选择端 $S=0$ 时 $Y=D_0$，$S=1$ 时 $Y=D_1$。$K$ 个选择端 → 在 $2^K$ 个数据输入中选一；大 MUX 可用 2 选 1 树搭建。

## 22. 系统实现策略（Systematic Implementation Strategies）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b2767c3a78d02aaf2a35845a76cbdcbd_Slide23.png" alt="MUX as systematic implementation" width="80%"/>

MUX 是实现逻辑的优雅通法：把函数输入接到选择端，把真值表输出列接到数据端常量。改功能只需改常量，不必重设计复杂 SOP。这是**可编程逻辑**的思路——制造后由用户配置功能；现代器件可替代数百万门，利于原型。$N$ 选择端 MUX 可代替任意 $N$ 输入逻辑，但数据端 $2^N$ 个，$N$ 很大时面积爆炸；实用约到 5–6。MUX 也是万能的；分子尺度逻辑中或许更自然。XOR 用单个 2 选 1 MUX 即可。

## 23. 查表综合（Synthesis By Table Lookup）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e8cb287e8ffbef2ae39101a46788f1cf_Slide24.png" alt="Synthesis By Table Lookup" width="80%"/>

（承上）MUX 本质是可编程查表；适合少输出列。多输出、同输入集合时，更适合下一节的 ROM——有限状态机中很常见。

## 24. 只读存储器（Read-only Memory, ROM）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3e8009230ca64f1fb4978402fa50227f_Slide25.png" alt="Read-only Memory" width="80%"/>

ROM 核心之一是**译码器**（decoder）：$K$ 个选择端、$2^K$ 个数据输出；任意时刻恰有一个输出为 1，由选择端的二进制值决定第 $J$ 路。

## 25. ROM 示例（ROM Example）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6ef6dc99067c529a9bd21796e34da9ad_Slide26.png" alt="ROM Example" width="80%"/>

以 2 输出真值表（如全加器）为例：$A,B,CI$ 接 3-to-8 译码器选择端；8 条水平线对应各输入组合。竖直列为各输出；NFET 下拉开关矩阵：某开关导通则该列被拉到地（LOW）；若无下拉则为 HIGH；再经反相得最终输出。某输入组合使唯一译码输出 HIGH → 打开该行开关 → 写出该行输出。改开关位置即可编程任意 3 入 2 出函数。

## 26. ROM 示例（续）（ROM Example continued）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9204e739c9689024cd246dfa2d3b2364_Slide27.png" alt="ROM Example continued" width="80%"/>

例：输入 001 时，标为 001 的译码线 HIGH，拉低 $S$ 列而不拉 $C_{\mathrm{OUT}}$，经输出反相后得 $S=1$、$C_{\mathrm{OUT}}=0$。

## 27. 更快的 ROM（Faster ROMs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0e02098920ca3c21527b477848346030_Slide28.png" alt="Faster ROMs" width="80%"/>

输入很多时，译码输出多、竖直列很长很慢。可把部分输入给译码器，其余输入用 MUX 在多段较短较快的竖直列之间选择——**小译码器 + 输出 MUX** 是常见做法。

## 28. 从 ROM 看逻辑（Logic According to ROMs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a09ef79c448ccdf78662eff7d0aaeda5_Slide29.png" alt="Logic According to ROMs" width="80%"/>

ROM 策略基本不看布尔结构：规模由输入/输出个数决定。开关矩阵常全填充，再物理/电气编程决定哪些受控、哪些永久关断。$N$ 入 $M$ 出 → 矩阵约 $2^N$ 行、$M$ 列，与真值表同阶。输入变化时译码线翻转时序略有差异，输出可能多次抖动才稳定——ROM **不宽容**，可能有毛刺。

## 29. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/393670768330460051c844e55b1e592b_Slide30.png" alt="Summary" width="80%"/>

| 策略 | 特点 |
|------|------|
| **积之和 + 反相逻辑** | 按具体函数定制；可做得又快又小；适合高性能或大批量 |
| **MUX / ROM** | 结构大体与具体函数无关；功能靠后续编程；适合原型、小批量、或出厂后需更新功能 |

本讲是组合逻辑实现技术的速览：从规格到 SOP，再到化简、K-map、MUX 与 ROM。
