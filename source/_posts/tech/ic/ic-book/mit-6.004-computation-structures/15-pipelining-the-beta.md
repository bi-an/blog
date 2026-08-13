---
title: MIT 6.004：L15 Beta 流水线
date: 2026-08-11 10:15:00
categories: ic
tags:
  - ic
  - digital-circuit
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L15 注解幻灯片。
>
> 源网页：[15.1 Annotated Slides | Pipelining the Beta](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c15/c15s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L15：Beta 流水线（Pipelining the Beta）

本讲把早先的电路流水技术用到 32 位 Beta：经典 **5 级流水**（IF/RF/ALU/MEM/WB），并用 **stall**、**bypass（forwarding）**、**speculation** 处理 **data hazard** 与 **control hazard**，以及异常/中断下的正确性。

## 1. 回顾：单周期 Beta（Reminder: Single-Cycle Beta）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3850be67dd37bf02d59a8657a467023f_Slide02.png" alt="Reminder Single-Cycle Beta" width="80%"/>

单周期 Beta 每拍执行一条指令：周期初装入新 PC → 取指 → 译码控制 → 读寄存器 → ALU；访存类再用 ALU 结果当地址，LD 的数据在周期末写回寄存器文件；也可写回 PC+4 或 ALU 结果。$t_{\mathrm{CLK}}$ 由整条执行路径累计延迟决定。问题：如何更快？

## 2. 单周期性能（Single-Cycle Beta Performance）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/97fc9be337084ab2e795d45e2cbbafad_Slide03.png" alt="Single-Cycle Beta Performance" width="80%"/>

程序时间 ≈（动态指令数）×（CPI）×（$t_{\mathrm{CLK}}$）。CPU 设计者主要能动 **CPI** 与 $t_{\mathrm{CLK}}$；改指令数需动 ISA 或编译器。单周期 Beta 的 CPI=1，但 $t_{\mathrm{CLK}}$ 取最坏路径：LD 需 $t_{\mathrm{IFETCH}}+t_{\mathrm{RF}}+t_{\mathrm{ALU}}+t_{\mathrm{MEM}}+t_{\mathrm{WB}}$。简单指令也被拖慢。是否让复杂指令多拍、简单指令一拍？本讲用流水重叠执行来提吞吐。

## 3. 流水实现（Pipelined Implementation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0d807936a1364a2af75478c99c28171e_Slide04.png" alt="Pipelined Implementation" width="80%"/>

把执行拆成多级、每级少几个部件 → 时钟可更短；多条指令重叠 → **吞吐**提高。单条**延迟**可能略增，但理想下每拍仍完成一条指令的末级。经典 **5 级**：

| 级 | 作用 |
|----|------|
| **IF** | 按 PC 取指 |
| **RF** | 读寄存器操作数 |
| **ALU** | 运算 |
| **MEM** | LD/LDR/ST 二次访存；非访存则旁路 ALU 结果 |
| **WB** | 写回目的寄存器 |

## 4. 为何不是 20 分钟讲完？（Why Isn’t This a 20-Minute Lecture?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3cac77f6446016f7f9424d312d7205a0_Slide05.png" alt="Why Isnt This a 20-Minute Lecture" width="80%"/>

组合电路流水：画轮廓、交叉处插流水寄存器即可。但 CPU **有状态**（寄存器/存储器），后级结果会影响前级（如 WB 写寄存器文件影响 RF 读）——存在指令间依赖，须专门处理。

## 5. 流水线冒险（Pipeline Hazards）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e5073dfeaddd7cf947570fdb55ec8511_Slide06.png" alt="Pipeline Hazards" width="80%"/>

两类问题依赖：

- **Data hazard**：当前指令要用更早指令产生的数据（如读 R0 依赖先前写 R0）
- **Control hazard**：分支/跳转/异常改变执行顺序

当被依赖指令仍在流水线中即触发 hazard。计划：先做无 hazard 序列正确的 5 级流水 → 修 data hazard → 再修 control hazard。

## 6. 简化单周期数据通路（Simplified Unpipelined Beta Datapath）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/87168bb73f48a15164ddd93db67d4dce_Slide07.png" alt="Simplified Unpipelined Beta Datapath" width="80%"/>

为便于加流水：先只谈顺序执行，去掉分支地址与 PC MUX（总是 PC+4；control hazard 时再加回）。寄存器文件画两次：上方组合读口（RF），下方时钟写口（WB）——物理上仍是同一组 32 个寄存器。

## 7. 五级流水数据通路（5-Stage Pipelined Datapath）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9aa791c376d1c9fe80b31916238879d0_Slide08.png" alt="5-Stage Pipelined Datapath" width="80%"/>

插入流水寄存器后，无 data hazard 时信息自上而下流动，重叠正确。每拍五级各处理不同指令。数据访存可跨近两拍启动/返回；存储器本身也可流水，同时结束上一访问并开始下一访问。控制逻辑如何按级拆分？

## 8. 流水控制（Pipelined Control）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/19da6c8f5959a8c166f32e4d7e34726c_Slide09.png" alt="Pipelined Control" width="80%"/>

每级带**指令寄存器**，由本级 opcode 产生本级控制；编码指令随流水向前传。RF 需 RA/RB/literal，WB 需 RC。逻辑与单周期类似，只是拆到各级；处理 hazard 时还要加额外控制。

## 9. 流水执行例（Pipelined Execution Example）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6d1ad231b92dab31dfab3b36b4f10514_Slide10.png" alt="Pipelined Execution Example" width="80%"/>

六条指令读写不同寄存器、无分支 → 无潜在 data/control hazard，可安全重叠。逐步跟踪。

## 10. 例：第 1 拍（Example: Cycle 1）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/cc3f6c826f9d3b14013cdf03f96113c0_Slide11.png" alt="Example Cycle 1" width="80%"/>

IF 用 PC 取绿色 LD，周期末写入 RF 级指令寄存器；同时算 PC+4（下一蓝指令地址）。用颜色标注各级正在处理的指令。

## 11. 例：第 2 拍（Example: Cycle 2）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5c1a2ee3389b34c932af5f85b3d9264e_Slide12.png" alt="Example Cycle 2" width="80%"/>

RF：绿指令读 R1；LD 使 ASEL=0、BSEL=1，选操作数写入 A/B 寄存器。IF 同时取蓝指令并更新 PC。

## 12. 例：第 3 拍（Example: Cycle 3）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b18a57a882e3764f2dc7ec928ad68dfb_Slide13.png" alt="Example Cycle 3" width="80%"/>

绿指令在 ALU：R1+4，结果写入 Y_MEM。

## 13. 例：第 4 拍（Example: Cycle 4）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/890641411703e484aa4b02a5815ee747_Slide14.png" alt="Example Cycle 4" width="80%"/>

四条指令重叠。MEM 为绿 LD 启动读；读数据要到 WB 才对 CPU 可用，本拍尚不可用。

## 14. 例：第 5 拍（Example: Cycle 5）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/13fd4195319af831f2493782074b1e38_Slide15.png" alt="Example Cycle 5" width="80%"/>

WB 把上拍启动的读数据写入 R2，绿 LD 完成。MEM 同时为蓝 LD 启动读。单指令延迟 5 拍，吞吐 1 指令/拍——与单周期同 CPI，但 $t_{\mathrm{CLK}}$ 更短。注意：R2 新值在第 5 拍末上升沿才写入，第 6 拍起才对其它指令可见——这就是 data hazard 的温床。

## 15. 流水线图（Pipeline Diagrams）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d44f640918afa82f75a29a5ed1392964_Slide16.png" alt="Pipeline Diagrams" width="80%"/>

数据通路图每拍要一张；更紧凑的是流水线图：行=流水级，列=周期，格内为指令。正常时指令沿对角线穿过五级。读寄存器在 RF，写在 WB 末。例：首条 LD 第 2 拍读 R1，第 5 拍末写 R2。

## 16. Data Hazard（Data Hazards）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/27d21b2b27e745bbd7d70ed4b5768874_Slide17.png" alt="Data Hazards" width="80%"/>

`ADDC` 写 R2，紧接 `SUBC` 读 R2——**read-after-write**。ADDC 第 5 拍末才写 R2，SUBC 第 3 拍已在 RF 读 → 读到旧值。流水结果须与单周期语义一致，必须修复。

## 17. 解决冒险策略 I（Resolving Hazards I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c3132fb9889829e2daa7b7fffed504c5_Slide18.png" alt="Resolving Hazards I" width="80%"/>

三种通用策略：

1. **Stall**：在 RF 卡住直到依赖满足；更早各级一并停。可靠但伤吞吐
2. **Bypass / forwarding**：结果已在后级数据通路中则直接前递，常可免 stall
3. **Speculation**：先猜，猜错再回退；适合 control hazard

## 18. 用 Stall 解 Data Hazard（Resolving Data Hazards I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/80c6defccc3ea098405ff883bb217598_Slide19.png" alt="Resolving Data Hazards I" width="80%"/>

SUBC 在 RF stall 三次，到第 6 拍才从寄存器文件读到新 R2；IF 同步 stall。RF 停时向 ALU 塞入 **NOP**（如目的为 R31 的 OP/OPC）——流水中的 **bubble**。检测：比较 RF 的 RA/RB 与 ALU/MEM/WB 的 RC（注意：有的指令不读两寄存器；ST 的 RC 含义不同；R31 恒可“匹配忽略”）。Stall 提高有效 CPI。

## 19. Stall 逻辑（Stall Logic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4078d53293cf02d4a1d4e0b43d170066_Slide20.png" alt="Stall Logic" width="80%"/>

`STALL=1`：禁止 IF/RF 输入流水寄存器装载；MUX 向 ALU 送 NOP，否则送当前 RF 指令。硬件不多；权衡是 CPI↑ vs $t_{\mathrm{CLK}}$↓。

## 20. 用 Bypass 解 Data Hazard（Resolving Data Hazards II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fafe30aea40fde1e45456488f746751a_Slide21.png" alt="Resolving Data Hazards II" width="80%"/>

ADDC 在第 3 拍 ALU 已算出将写入 R2 的值，恰可供给同拍 RF 中的 SUBC。若 RF 的源寄存器号匹配 ALU 的 RC，用 ALU 输出代替寄存器文件陈旧读值——红箭头即 bypass。

## 21. Bypass 逻辑（Bypass Logic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b57d0c86c4f1af9cb7ec940673eba7ab_Slide22.png" alt="Bypass Logic" width="80%"/>

在读口加多路 MUX，可从 ALU/MEM/WB 前递。多路同时匹配时选**最近**指令：优先 ALU，再 MEM，再 WB，最后才是寄存器文件（注意 R31）。

## 22. 全 Bypass 流水线（Fully Bypassed Pipeline）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/33ff1debe131484d5a96ae0cb5b598cf_Slide23.png" alt="Fully Bypassed Pipeline" width="80%"/>

分支/跳转写回 PC+4，故 PC+4 路径也要 bypass。前递发生在周期末（如 ALU 算完后），MUX 的 $t_{\mathrm{PD}}$ 略拉长 $t_{\mathrm{CLK}}$。可折中：只 bypass ALU 级结果，其余靠 stall。全 bypass 后还要不要 STALL？

## 23. Load-to-Use Stall（Load-to-Use Stalls）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1abefdac725dd9dff7983b5eabdcb12a_Slide24.png" alt="Load-to-Use Stalls" width="80%"/>

**Load-to-use**：紧接使用 LD 结果。LD 数据要到 WB 才在通路中可用，即便全 bypass，SUBC 仍须在 RF stall（例中 stall 到第 5 拍，插入 2 个 NOP）；若无 WB bypass 则更久。

## 24. 小结：带 Data Hazard 的流水（Summary: Pipelining with Data Hazards）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/943e3a78f160c335a0a6c5cc39e1aaec_Slide25.png" alt="Summary Pipelining with Data Hazards" width="80%"/>

Stall：硬件简单，bubble 抬高 CPI。Bypass：硬件更多，一般不抬 CPI；但仍须 stall 处理 load-to-use。级数越多同拍在飞指令越多，hazard/stall 更频繁，CPI 压力更大。

## 25. 编译器可帮忙（Compilers Can Help）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7bd3fa7e49473814544836ea1cfe8eb7_Slide26.png" alt="Compilers Can Help" width="80%"/>

重排无关指令可拉开 load-to-use 距离：把独立的 MUL/XOR 挪到 SUBC 前，使 LD 到 WB 时使用方才到 RF → 零 stall。前提是找得到可移动的独立指令。

## 26. 懒办法：改 ISA（Or Take the Lazy Route…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/07584131e6cb828a99fb925c86f45afc_Slide27.png" alt="Or Take the Lazy Route" width="80%"/>

把“写回延迟 3 条指令”写进 ISA，让程序员/编译器显式插 NOP——硬件省、软件苦；改流水深度还得再改 ISA。成功 ISA 寿命长，不宜绑定短期实现折中。

## 27. Control Hazard I（Control Hazards I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f8007f709c40423dd2edf0fbb3e24161_Slide28.png" alt="Control Hazards I" width="80%"/>

`BNE` 后执行谁取决于 R3 是否非零。显式控制转移使下一条依赖当前指令执行结果——对流水意味着什么？

## 28. Control Hazard II（Control Hazards II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9b2a155a4d8f52930cdfaece0ee9bb80_Slide29.png" alt="Control Hazards II" width="80%"/>

分支下一 PC 依赖 opcode、当前 PC（算偏移）与 RA；JMP 依赖 opcode 与 RA；其它指令多为 PC+4。异常也改 PC（后文）。问题在于：JMP/分支在 RF 才读到 RA（bypass 可保证 RA 值正确），但同拍 IF 已在取“下一条”——取谁？

## 29. 解决 Control Hazard（Resolving Control Hazards）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/23c68b52d91dc0cdb187780ff0b6a804_Slide30.png" alt="Resolving Control Hazards" width="80%"/>

对 JMP 与 taken 分支，IF 在 RF 算出目标前不知道该做什么。一策：stall IF 直至 RF 算完。

## 30. 用 Stall 解 Control Hazard（Resolving Control Hazards with Stalls）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/de92a047f3a1ae1d127bd126f5ebcf2e_Slide31.png" alt="Resolving Control Hazards with Stalls" width="80%"/>

RF 中为 JMP/BEQ/BNE 时 stall IF 一拍，插入 NOP；RF 确定目标后再继续。例中循环还叠加 data hazard，靠 bypass 从 MEM 取 R3。3 指令循环实际 4 拍一轮 → 有效 CPI=$4/3$（约 +33%）。

## 31. Control Hazard 的 Stall 逻辑（Stall Logic for Control Hazards）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fb0472b32f2e637b83e8d5aad7cb929f_Slide32.png" alt="Stall Logic for Control Hazards" width="80%"/>

IF 指令路径加 MUX，由 `IRSrc_IF` 控制：RF 为 JMP/BEQ/BNE 时选 NOP，**annul** 刚取出的指令；同时 `PCSEL` 选正确下一 PC。上标表示控制逻辑所在流水级。

## 32. ISA：简单 vs 复杂分支（ISA Issues: Simple vs. Complex Branches）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f5056e1da7ebfd2395b9775506b28cd4_Slide33.png" alt="ISA Issues Simple vs Complex Branches" width="80%"/>

Beta 分支在 RF 决策。若 ISA 把决策放到 ALU，须 annul IF 与 RF 两条 → 两 NOP，CPI 更差；但复杂分支可能减少静态指令数。提前各级全部 annul 称 **flush**——代价大，仅在别无他法保正确时用。

## 33. 解决冒险策略 II：推测（Resolving Hazards II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ba0492658cebeefe776450d12b9a31ba_Slide34.png" alt="Resolving Hazards II" width="80%"/>

未 taken 时，流水本来按 PC+4 取指就是对的。在不确定时仍开始执行称 **speculation**；须能在产生副作用（写寄存器/主存）前 annul。副作用在后级，故指令可先走过 IF/RF/ALU 再最终决定。

## 34. 推测 I（Resolving Hazards with Speculation I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/350c4133a5a1717739f6b828adfc7a0c_Slide35.png" alt="Resolving Hazards with Speculation I" width="80%"/>

默认猜下一 PC=PC+4，仅对 JMP/taken 分支错。若 BNE 未 taken：后续 SUB 进入流水，第 4 拍末确认后放行——猜对则零 annul。

## 35. 推测 II（Resolving Hazards with Speculation II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/abcd76354dd4bdf12990ec04b18b8138_Slide36.png" alt="Resolving Hazards with Speculation II" width="80%"/>

若 BNE taken：第 4 拍末 annul SUB，第 5 拍执行 NOP。仅 taken 时插入 bubble，对 CPI 冲击小于“分支一律 stall”。

## 36. 推测控制逻辑（Speculation Logic For Control Hazards）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/89fefa9e9395a8e72e7a089d97207ecd_Slide37.png" alt="Speculation Logic For Control Hazards" width="80%"/>

数据通路同前，只是更聪明地置 `IRSrc_IF=1`：不是所有分支，仅 **taken** 时 annul IF。

## 37. 分支预测（Branch Prediction）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5d32c1ef60fa9fa1fcb7397dee5b31a3_Slide38.png" alt="Branch Prediction" width="80%"/>

总猜 PC+4 对 JMP/taken 常错，仿真约抬高有效 CPI ~10%。深流水（如 Nehalem）分支很晚才决出，flush 代价极大。现代做法：非分支仍猜顺序；分支按历史、循环反向偏移、甚至分支间相关做预测，正确率可达 95%–99%。

## 38. 分支延迟槽 I（Branch Delay Slots I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/71f99872626bd816b1b69f8a15bf8ab9_Slide39.png" alt="Branch Delay Slots I" width="80%"/>

改 ISA：跳转/分支后的下一条**总是执行**（延迟槽）。则猜 PC+4 恒对。把循环中 MUL 放到 BNE 后的 delay slot，可零 CPI 惩罚（若填得满）。

## 39. 分支延迟槽 II（Branch Delay Slots II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4f0dba69b63ad9a9714efac99f189e05_Slide40.png" alt="Branch Delay Slots II" width="80%"/>

实践中约一半情况找不到有用指令填槽，只好显式 NOP，代码变大；决策越晚槽越多越难填。分支预测通常优于 delay slot。再次：勿为某一实现改 ISA 语义。

## 40. 异常（Exceptions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/244e40cd5f23c805c6dc312bcb7c1269_Slide41.png" alt="Exceptions" width="80%"/>

非法指令或外部中断：存 PC+4 到 XP，PC←相应 handler。异常是隐式控制转移。单周期中异常作用于“当前指令”；流水中须认定**哪一条**受影响，保证更早指令完成，并 annul 该指令及其后已进流水者。

## 41. 异常何时发生？（When Can Exceptions Happen?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c0582b71f660b770a96e43bef064dc4b_Slide42.png" alt="When Can Exceptions Happen" width="80%"/>

RF：非法 opcode；ALU：如 DIV 除 0；MEM：非法地址；IF：取指地址异常。后续已进流水的指令须 annul。好消息：寄存器只在 WB 更新，annul 只需换成 NOP，不必回滚已写值。

## 42. 处理异常（Resolving Exceptions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c9abff0ec412841ffac3e6fae3502e2b_Slide43.png" alt="Resolving Exceptions" width="80%"/>

若指令在第 $i$ 级引发异常：用一条副作用仅为把 PC+4 写入 XP 的“魔法” **BNE** 替换它 → flush 更早各级 → PC←handler。例：LD 在第 4 拍 MEM 异常，第 5 拍起 IF 取 handler。

## 43. 异常处理逻辑（Exception Handling Logic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fb5877e66fe5c3db57955ec82142f1d5_Slide44.png" alt="Exception Handling Logic" width="80%"/>

改造指令路径 MUX：可换成 NOP（annul）或魔法 BNE（肇事指令）。

## 44. 多重异常？（Multiple Exceptions?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/55509ce447473e9a3391e5945b35d6cb_Slide45.png" alt="Multiple Exceptions" width="80%"/>

多指令并行时可能同时/先后检出多个异常。例：非法 opcode 在 RF 先被发现，但更早的 LD 随后在 MEM 也异常——应让**更早指令**（流水中更靠后级）的异常优先，因放弃 LD 后其后指令本就不该执行。同拍多异常：优先流水线中更靠前（更接近完成）的那条。

## 45. 异步中断（Asynchronous Interrupts）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7d296169a216e179459ab718d9537578_Slide46.png" alt="Asynchronous Interrupts" width="80%"/>

外部中断也像隐式分支，但可当作作用于 **IF** 的异常：用魔法 BNE 捕获 PC+4，下一 PC←中断 handler；handler 返回前修正 XP 指向被打断指令（如 SUB）。更早的 ADD/LD 等不受影响。

## 46. 异常+中断逻辑（Exception + Interrupt Handling Logic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dd479d83611a87939bde3ab873296c31_Slide47.png" alt="Exception Interrupt Handling Logic" width="80%"/>

沿用指令路径 MUX；调整 `IRSrc_IF`：有中断请求时也为 1。

## 47. 五级 Beta 最终版（5-Stage Beta: Final Version）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f50097acd4debaf7bdfa8c1503d0916f_Slide48.png" alt="5-Stage Beta Final Version" width="80%"/>

汇总：IF/RF stall + 读口 bypass MUX + stall 时向后塞 NOP；控制上默认推测 PC+4，JMP/taken 时 annul IF；异常/中断在除 WB 外各级可换指令（肇事→魔法 BNE，更早→NOP）。额外电路保证流水语义≡单周期；bypass 与分支预测使 hazard 对有效 CPI 冲击有限，短时钟换来大幅吞吐提升。

## 48. 回顾：解决冒险（Reminder: Resolving Hazards）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bb23ace0a541501f185def90e02674d5_Slide49.png" alt="Reminder Resolving Hazards" width="80%"/>

记住三板斧：**stall**、**bypass**、**speculation**。高性能流水设计几乎总能落到其中之一。流水讨论至此；更高性能的其它途径（并行等）留待后续讲座。
