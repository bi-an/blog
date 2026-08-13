---
title: MIT 6.004：L08 设计折衷
date: 2026-08-11 10:08:00
categories: ic
tags:
  - ic
  - digital-circuit
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L08 注解幻灯片。
>
> 源网页：[8.1 Annotated Slides | Design Tradeoffs](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c8/c8s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L08：设计折衷（Design Tradeoffs）

本讲讨论功耗（静态/动态）、降耗手段，以及加法器（ripple / carry-select / CLA）与乘法器（组合 / 流水 / 时序 carry-save）在延迟、吞吐与面积上的 $\Theta$ 折衷。完成本课程 Part 1 的收束。

## 1. 优化你的设计（Optimizing Your Design）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/02fd721dc2dd430ee926f004e40b92c6_Slide02.png" alt="Optimizing Your Design" width="80%"/>

正确折衷取决于设计目标。显卡团队：优先性能，可在限度内换成本与功耗；尺寸有硬上限，再小收益不大。手表团队：尺寸与功耗关键（戴一天、不烫手腕）。

同一“是否流水”决策：流水寄存器增加成本；重叠执行与更高 $t_{\mathrm{CLK}}$ 抬高功耗与散热需求 → 两队结论可能相反。本章列举可用折衷；选对约束下的权衡是工程师的乐趣。

## 2. CMOS 静态功耗（CMOS Static Power Dissipation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/345c19093c798eff3766c490848fd977_Slide03.png" alt="CMOS Static Power Dissipation" width="80%"/>

**静态功耗**：电路空闲（节点不变）时仍消耗的功率。理想开关模型下 CMOS 静态功耗为 0；早期接近理想，但尺寸缩小、电压降低后两大效应凸显（n/p 沟道皆有）：

1. **栅氧变薄** → 电场增强、速度提升，但电子可隧穿绝缘层，产生栅→衬底漏电流；数十亿管累积不可忽视
2. **亚阈导通（sub-threshold conduction）**：名义截止（$V_{\mathrm{GS}}<V_{\mathrm{TH}}$）时仍有漏电，与 $V_{\mathrm{GS}}-V_{\mathrm{TH}}$（截止时为负）指数相关；$V_{\mathrm{TH}}$ 降低使漏电增大

缓解：FinFET / tri-gate（沟道成鳍、栅三面包围）可把亚阈漏电降一个数量级以上。

## 3. CMOS 动态功耗 I（CMOS Dynamic Power Dissipation I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b98de863ffa12f5f6cc156110401efa1_Slide04.png" alt="CMOS Dynamic Power Dissipation I" width="80%"/>

**动态功耗**：节点翻转充放电电容时消耗。以反相器为例：充/放电时电流经 MOSFET，瞬时功率 $I_{\mathrm{DS}}V_{\mathrm{DS}}$。对 1→0 积分（$I=C\,dV/dt$），若时钟周期 $t_{\mathrm{CLK}}$、每半周期一次翻转，经 pulldown 耗散的功率约为 $0.5\,f\,C\,V_{\mathrm{DD}}^2$（$f$ 为每秒翻转次数）。

完整充→放一周期耗散 $CV_{\mathrm{DD}}^2$ 焦耳；频率 $f$ 时平均功率为 $fCV_{\mathrm{DD}}^2$ 瓦；能量全部来自电源：一半充电容时耗散，一半存于电容后放电耗散。

## 4. CMOS 动态功耗 II（CMOS Dynamic Power Dissipation II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/37e8cd20bb45b2a95b118db5e26eeb42_Slide05.png" alt="CMOS Dynamic Power Dissipation II" width="80%"/>

整电路：每周期约 $N$ 个节点翻转时，可据此估算总能耗。信封估算：1 GHz、1 亿内部节点、每节点 $\sim 1\,\mathrm{fF}$、$V_{\mathrm{DD}}\sim 1\,\mathrm{V}$ → 约 **100 W**——接近白炽灯泡，散热困难。笔记本 CPU 远低于此，靠设计技巧压功耗。

单独降低 $V_{\mathrm{DD}}$（如 3.3 V→1 V）可降功耗一个数量级以上（因 $\propto V^2$）。技术趋势使晶体管更多更快，若不精打细算会撞上功耗墙。

## 5. 如何降低功耗？（How Can We Reduce Power?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3d22a73e648d2472e0f2feb9870983e2_Slide06.png" alt="How Can We Reduce Power" width="80%"/>

ALU 常含算术、布尔、移位、比较等独立模块；控制只选其一，其余结果被忽略——但仍在耗动态功耗。机会：关掉不需要的模块。

思路：阻止其输入变化 → 内部节点不动 → 动态功耗为零。

## 6. 更少翻转 → 更低功耗（Fewer Transitions → Lower Power）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/633c6597c50b97eb31e1f3b768cecce1_Slide07.png" alt="Fewer Transitions Lower Power" width="80%"/>

手段：

- 各模块输入加 **latch**，仅当本周期需要该结果时打开 → 移位器等大模块多数时间不翻转
- **切断电源**关断整块（更复杂，常用于特殊省电模式）
- **降频**：无事时减慢时钟（对毫秒级外部事件尤其有效），有事件再加速

移动设备广泛使用上述技巧。计算是否必然耗能见课程笔记 §6.5。

## 7. 提速：加法器例子（Improving Speed: Adder Example）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/51a031cceb0374062aceca0b3802dccf_Slide08.png" alt="Improving Speed: Adder Example" width="80%"/>

瓶颈常在**行波进位加法器（ripple-carry）**：最长路径是进位链。触发：A 全 1、B 最低位 1 其余 0（加 $-1$ 与 $1$）→ 进位逐级 ripple。

延迟 $\approx (N-1)\times t_{\mathrm{carry}}+t_{\mathrm{sum}}$。$N$ 翻倍则延迟约翻倍 → 延迟 $\Theta(N)$（阶记号忽略相对次要项）。

## 8. 性能/成本分析（Performance/Cost Analysis）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4c0a6a6a7d6f131cb7c8f764b984c337_Slide09.png" alt="Performance/Cost Analysis" width="80%"/>

**渐近分析**：标出 $N\to\infty$ 时主导项。例：$n^2+2n+3$ 被常数倍的 $n^2$ 上下夹住（除有限个 $n$）→ $\Theta(n^2)$。

- $\Theta(f)$：上下皆被 $f$ 的常数倍界定
- $O(f)$：仅上界

## 9. 进位选择加法器（Carry-Select Adders）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b4eec9a4dbe07ba8c021e53792e49da2_Slide10.png" alt="Carry-Select Adders" width="80%"/>

高半部不必等低半部进位：做两份高 16 位加法（假设 $C_{\mathrm{in}}=0$ 与 $1$），与低半部并行；再用低半部真实进位选结果 → **carry-select adder**。

32 bit 延迟约略高于 16 bit ripple，$\sim$ 减半延迟，代价约 +50% 电路。递归对更小块做同样事：$\log_2 N$ 层后延迟为常数加法 + $\log N$ 个 MUX → **$\Theta(\log N)$**。清晰的性能–面积折衷。

## 10. 32 位进位选择加法器（32-Bit Carry-Select Adder）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/46ed3f71f5d694c186d68678f68b0f21_Slide11.png" alt="32-Bit Carry-Select Adder" width="80%"/>

工程版：块大小使试算和与前级进位几乎同时到达 select MUX；select 负载大时加 buffer。相对 32 bit ripple：**约 2.5× 更快，约 2× 电路**——ALU 提速常记方案。

## 11. 要更快的进位逻辑！（Wanted: Faster Carry Logic!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/95254d02690b7d5ffe1b604bf8c2f76f_Slide12.png" alt="Wanted: Faster Carry Logic" width="80%"/>

链 → 树可把线性延迟变对数。改写全加器进位：

- **G（generate）**：无需等 $C_{\mathrm{in}}$ 即产生进位
- **P（propagate）**：有 $C_{\mathrm{in}}$ 才传出

$C_{\mathrm{out}}=G\lor(P\land C_{\mathrm{in}})$。常把 P 从 $A\lor B$ 改为 $A\oplus B$，则和可写 $S=P\oplus C_{\mathrm{in}}$。进位 SOP 可用三个 2 输入 NAND 实现。

## 12. 超前进位加法器（Carry Look-Ahead Adders, CLA）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/70c97b96738a16c96f129da9774b1058_Slide13.png" alt="Carry Look-Ahead Adders" width="80%"/>

相邻两模块 H、L：块 generate / propagate：

$$
G_{\mathrm{block}}=G_H\lor(G_L\land P_H),\quad P_{\mathrm{block}}=P_L\land P_H.
$$

用 GP 模块组合两级进位信息，把两模块当作更大块。

## 13. 8 位 CLA（生成 G 与 P）（8-Bit CLA: generate G & P）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9ee781059dbfc183473eb773cb614b89_Slide14.png" alt="8-Bit CLA generate G and P" width="80%"/>

多层 GP 成树：$N$ 输入共 $N-1$ 个 GP 模块，延迟 $\Theta(\log N)$。下一步用 G/P 快速算各全加器的 $C_{\mathrm{in}}$。

## 14. 8 位 CLA（进位生成）（8-Bit CLA: carry generation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2852c46e1a334eea1fa84f94375ddc51_Slide15.png" alt="8-Bit CLA carry generation" width="80%"/>

给定最低位 $C_0$：块的低半直接用该进位；高半进位用低半 G/P 算出。C 模块再排成树，层层算出各全加器 $C_{\mathrm{in}}$，延迟仍 $\Theta(\log N)$。注意同一位置 C 的 $G_L,P_L$ 与 GP 树输入对应。

## 15. 8 位 CLA（完整）（8-Bit CLA: complete）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fb004b8363ed061d385f690330be17ca_Slide16.png" alt="8-Bit CLA complete" width="80%"/>

GP 与 C 合成 **carry-lookahead 模块**：向上传 G/P，向下传进位。上行+下行总延迟 $\Theta(\log N)$，再加一级 XOR 得和 → 整加法器延迟 $\Theta(\log N)$，远优于 ripple 的 $\Theta(N)$。全加器内原 carry-out 逻辑可删。同类策略的极致见 Kogge-Stone 等。

## 16. 二进制乘法（Binary Multiplication）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6d9abf5815abc53e2a8c95d70638c091_Slide17.png" alt="Binary Multiplication" width="80%"/>

小学竖式：乘数每位 × 被乘数得**部分积**，左移对齐后相加。1 bit×1 bit 即 **AND**，无进位；部分积宽 $N$。$M$ 位乘数 → $M$ 个部分积；相加得 $N+M$ 位结果。贵在把 $M$ 个 $N$ 位部分积相加。

## 17. 组合乘法器（Combinational Multiplier）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/474405b2bc4c533af42b4f9ff7da3a86_Slide18.png" alt="Combinational Multiplier" width="80%"/>

$M\times N$ 个 AND 算部分积；行间用全加器/半加器累加。信息只向下或向左，最长路径至多 $N+M$ 个模块 → 延迟 $\Theta(N)$（$M,N$ 差常数倍）；吞吐 $=1/\textrm{latency}$；硬件 $\Theta(N^2)$。

## 18. 补码乘法（2’s Complement Multiplication）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5752935a18c90254f7a2f6f4c2cced9f_Slide19.png" alt="2's Complement Multiplication" width="80%"/>

补码最高位负权重 → 部分积需**符号扩展**到 $N+M$ 位；末个部分积因乘数符号位而改为**减**。技巧：在若干列加 1 再抵消，把符号扩展与减法化成：若干位取反 + 两处加常数 1——最终表几乎与无符号竖式同形。

## 19. 补码乘法器电路（2’s Complement Multiplier）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/41539f33e60418a55b8874e6c0c63f4f_Slide20.png" alt="2's Complement Multiplier" width="80%"/>

相对无符号版：部分 AND→NAND（取反），并改两处加 1 的逻辑。延迟、吞吐、硬件代价与无符号版同阶。

## 20. 流水提高吞吐（Increase Throughput with Pipelining）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/64941f00eca147e2afa39f8f33bab9f9_Slide21.png" alt="Increase Throughput with Pipelining" width="80%"/>

原组合乘吞吐约每 $2N$ 拍出一个结果。画输出轮廓得 1-pipeline；再对半切可翻倍吞吐，但仍 $\Theta(1/N)$。关键洞察：整行在同一级 → 级延迟仍含 $N$ bit ripple → $\Theta(N)$。

## 21. Carry-Save 流水乘法器（Carry-Save Pipelined Multiplier）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/90cfb1333b316167622017a4a36cb268_Slide22.png" alt="Carry-Save Pipelined Multiplier" width="80%"/>

重画进位链：进位仍向左一列，但接到**下一行**同列。水平流水线切断长进位；每级延迟常数（与 $N$ 无关）。需 $\Theta(N)$ 额外行收尾进位。

$\Theta(N)$ 级 → 级延迟/时钟/吞吐皆 $\Theta(1)$；系统延迟 $\Theta(N)$；硬件仍 $\Theta(N^2)$。吞吐显著提升的折衷。

## 22. 时序逻辑减面积（Reduce Area with Sequential Logic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dd318325bc7631c4be4a663b6d919631_Slide23.png" alt="Reduce Area with Sequential Logic" width="80%"/>

时序乘法器：每拍算一个部分积并累加到 P，共 $\Theta(N)$ 步。B 最低位 × 被乘数 → carry-save 加法器；P 与加法器输出为 **carry-save 格式**（数据和 + 保存的进位）。每拍 P、B 右移 1——等价于“部分积左移”的对偶。

无完整进位传播 → 时钟周期与 $N$ 无关（约一个全加器延迟）。再 $\Theta(N)$ 步收尾进位 → 总延迟仍 $\Theta(N)$，吞吐 $\Theta(1/N)$，**硬件 $\Theta(N)$**（相对组合 $\Theta(N^2)$ 大降）。Carry-save：可同硬件提吞吐，或同吞吐省面积。

## 23. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8012e7beaf1574bff538acf378456720_Slide24.png" alt="Summary" width="80%"/>

Part 1 收束：信息与编码 → 冗余检错/纠错 → 数字抽象与噪声容限 → MOSFET/CMOS 与组合综合 → 双稳态存储与动态纪律 → FSM 与亚稳态/同步 → 延迟与吞吐 → 本讲功耗与加法/乘法折衷。

设计时可在功耗、延迟 $\Theta(N)$ vs $\Theta(\log N)$、吞吐 $\Theta(1)$ vs $\Theta(1/N)$、面积 $\Theta(N)$ vs $\Theta(N^2)$ 之间按目标选型。
