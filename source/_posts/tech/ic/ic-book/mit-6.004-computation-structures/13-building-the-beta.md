---
title: MIT 6.004：L13 构建 Beta
date: 2026-08-11 10:13:00
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L13 注解幻灯片。
>
> 源网页：[13.1 Annotated Slides | Building the Beta](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c13/c13s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L13：构建 Beta（Building the Beta）

本讲增量搭建单周期 Beta：寄存器堆、ALU / 访存 / 分支 / LDR 数据通路，以及异常与中断；并汇总控制信号 ROM 方案。目标是“每时钟一条指令”的可工作 32 位 RISC。

## 1. CPU 设计权衡（CPU Design Tradeoffs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8739da650177b37a215297f046e4b2dd_Slide02.png" alt="CPU Design Tradeoffs" width="80%"/>

正确执行 Beta ISA 是底线；还要权衡：性能（常用 **MIPS** = Millions of Instructions Per Second）、芯片面积（成本）、性能/价格、性能/瓦特等。Intel 8080（1974）约 0.29 MIPS；现代多核可达 $10^4\sim 10^5$ MIPS。Apple Watch 与高端桌面目标集不同。

## 2. 处理器性能（Processor Performance）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/020855b152d04544e78a2e0b3c662df3_Slide03.png" alt="Processor Performance" width="80%"/>

执行时间 $\propto$ 程序指令数 $\times$ 每条平均时钟数 $\times$ 时钟周期。可减少指令数、降低 CPI、或缩短周期（简化逻辑）。本讲实现**每时钟一条指令**；组合路径较长，日后可用流水线提吞吐。

## 3. 回顾：Beta ISA（Reminder: Beta ISA）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/404d904ad2737e9c7c346f761861a84e_Slide04.png" alt="Reminder Beta ISA" width="80%"/>

32 个 32 位寄存器。两类主要格式：

- opcode 高 2 位 `0b10`：双寄存器操作数（Ra、Rb）→ Rc
- 高 2 位 `0b11`：第二操作数为 16 位常量（$-32768\sim 32767$）；助记符加 `C`；访存与分支也用此格式

操作含算术、比较、布尔、移位。仅两种格式 → 译码/控制简单，许多指令位可直接接到数据通路。

## 4. 方法：增量加功能（Approach: Incremental Featurism）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f7bf486389ed4c9bf512f2212a95d2dc_Slide05.png" alt="Approach Incremental Featurism" width="80%"/>

顺序：先 ALU 指令 → 访存与分支 → 异常。构件：多位寄存器（上升沿加载）、大量 MUX、Part 1 的 ALU、以及寄存器堆与主存。

## 5. 多端口寄存器堆（Multi-ported Register File）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6fb2df496c2ea1693df0af51048b3d6b_Slide06.png" alt="Multi-ported Register File" width="80%"/>

32 个带 EN 的寄存器：EN=1 时下一上升沿装入 D；**禁止门控时钟**（NO GATED CLOCKS）。两套读 MUX（地址 RA1/RA2）独立读；写口用 5 位 WA 译码选中一个 WE。R31 读出恒为 0。封装为双读一写寄存器堆。

## 6. 寄存器堆时序（Register File Timing）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b5463bafbd31e54e8b047562068d1644_Slide07.png" alt="Register File Timing" width="80%"/>

读：地址稳定后经 $t_{\mathrm{PD}}$ 出数据。写：WA/WD/WE 须满足建立/保持时间。同一周期可读旧值、周期末写入、下一周期才见新值——可同址同周期“读旧写新”。

## 7. ALU 指令（ALU Instructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7e96042fb66664215b3c435ddde7ed98_Slide08.png" alt="ALU Instructions" width="80%"/>

五步：**Fetch**（按 PC 取指）→ **Decode**（opcode→控制）→ **Read**（Ra/Rb）→ **Execute**（ALU + 下一 PC）→ **Write-back**（写 Rc）。时钟上升沿更新寄存器堆与 PC，标志当前指令结束。周期须覆盖五步总延迟；若 $t_{\mathrm{CLK}}=10\,\mathrm{ns}$ → 100 MHz → 100 MIPS。

## 8. 取指/译码（Instruction Fetch/Decode）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3910af72706e4c08fe212a045add03aa_Slide09.png" alt="Instruction Fetch Decode" width="80%"/>

PC → 主存取指；ALU 类下一地址为 PC+4（专用加法器）。RESET 时 MUX 选初始 PC。部分指令字段可直连；其余控制由 opcode 逻辑产生。

## 9. ALU 操作数据通路 I（ALU Op Datapath I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/03fd3f09e2add1349a292477fefdb996_Slide10.png" alt="ALU Op Datapath I" width="80%"/>

Ra/Rb/Rc 字段直连寄存器堆读写地址；读数进 ALU；ALUFN 由 opcode 经控制逻辑（可实现为 $2^6=64$ 项 ROM）给出。ALU 结果回写 Rc；**WERF**（write-enable register file）=1 时写入。课程吉祥物 Werf 即以此命名。

## 10. ALU 操作数据通路 II（ALU Op Datapath II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e804bf92068fcdae06daa5d9fca6dfc1_Slide11.png" alt="ALU Op Datapath II" width="80%"/>

取指后 Ra/Rb 读出 → ALUFN 选定 → 结果经 WERF=1 写回 Rc。RISC 优势：执行所需数据通路直观。

## 11. 带常量的 ALU I（ALU Operations with constant I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8d41919becb427fb16aebe97bd449981_Slide12.png" alt="ALU Operations with constant I" width="80%"/>

第二操作数 = 指令低 16 位符号扩展。加 **BSEL** MUX：0 选寄存器，1 选常量。符号扩展纯接线：复制 `ID[15]` 十六次，无需门。

## 12. 带常量的 ALU II（ALU Operations with constant II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c4774136a52743eccd7e91f8cf8510e4_Slide13.png" alt="ALU Operations with constant II" width="80%"/>

BSEL=1，其余同双寄存器 ALU。至此已能执行大部分 ISA；剩下访存与分支。

## 13. Load 指令 I（Load Instruction I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0bf8007cf2364904abaf9c9fd480132f_Slide14.png" alt="Load Instruction I" width="80%"/>

LD/ST 访问与指令同一主存（图上分画两盒）。地址计算同 ADDC：Ra + 符号扩展字面量，复用 ALU。LD：ALU 结果作地址；MOE=1 读出；经 **WDSEL** 三选一 MUX（另两路留给分支写回 PC+4 等）写回 Rc。MWE 用于写存。

## 14. Load 指令 II（Load Instruction II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/75c083b00fb31b15c42f32b059131193_Slide15.png" alt="Load Instruction II" width="80%"/>

操作数选法同 ADDC，ALU 做 ADD；WDSEL=2 选存返回数据写寄存器堆。

## 15. Store 指令 I（Store Instruction I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/00de86fbe80bf6f6c5fbb6e5f52f4199_Slide16.png" alt="Store Instruction I" width="80%"/>

要写的数据来自 Rc，但 Rc 未接读口；ST 不用 Rb → 加 **RA2SEL** MUX：1 时第二读口地址用 Rc。该读数进主存 WD。ST 不写寄存器堆 → WERF=0。

## 16. Store 指令 II（Store Instruction II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/14d6b0751ffec005312e336d2010fe6b_Slide17.png" alt="Store Instruction II" width="80%"/>

地址同 LD；MWR/MWE=1 在周期末写存；WERF=0；WDSEL 为 don't care（可任选便于逻辑最小化）。**注意**：MWE 绝不能是 don't care，以免误写。

## 17. JMP 指令 I（JMP Instruction I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5f216c8e4b03e67b64cc41789cf0f8a8_Slide18.png" alt="JMP Instruction I" width="80%"/>

**PCSEL** MUX 选下一 PC：0→PC+4，2→Ra。JMP/分支还把 PC+4 写入 Rc（WDSEL=0）。

## 18. JMP 指令 II（JMP Instruction II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/058b80b37e401bd6a10694e6d7b55c38_Slide19.png" alt="JMP Instruction II" width="80%"/>

WERF=1 写回 PC+4；PCSEL=2 选 Ra。其余多为 don't care，但 MWR 必须安全为 0。

## 19. BEQ/BNE 指令 I（BEQ/BNE Instructions I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/063544445b874078a1108103f9fd70c9_Slide20.png" alt="BEQ BNE Instructions I" width="80%"/>

偏移加法器：PC+4 +（字面量左移 2 位的符号扩展偏移）。乘 4 靠接线插两个 0；符号扩展复制 `ID[15]` 十四次。32 位 NOR 得 **Z**（Ra 全 0）。分支成立 → PCSEL=1 选目标；否则 PCSEL=0。

## 20. BEQ/BNE 指令 II（BEQ/BNE Instructions II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3760061cff104997306fe7aa897efd8e_Slide21.png" alt="BEQ BNE Instructions II" width="80%"/>

PC+4 写 Rc；Z 与偏移加法器结果共同决定 PCSEL。

## 21. 相对加载 LDR（Load Relative Instruction）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/82f59ccaf861bd6151ee2e0ae838acc0_Slide22.png" alt="Load Relative Instruction" width="80%"/>

LDR 像 LD，但地址来自分支偏移加法器——用于加载放在代码旁、装不下 16 位字面量的大常量。常量放过程后等位置，且须避免被当作指令执行。

## 22. LDR 指令 I（LDR Instruction I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1906ba8776a08d6f78845a85d1731af5_Slide23.png" alt="LDR Instruction I" width="80%"/>

加 **ASEL** MUX：1 时第一 ALU 操作数来自偏移加法器；ALU 做布尔 “A” 把该值送到地址口。为何不把 ASEL 直接接到存地址？那样 LD/ST 地址会多串一级 MUX，拉长关键路径、拖慢**所有**指令时钟；ASEL 与 BSEL 延迟重叠则几乎零性能代价。

## 23. LDR 指令 II（LDR Instruction II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/adbacfe6050fcf3f4ff98580e62fe9af_Slide24.png" alt="LDR Instruction II" width="80%"/>

偏移 → ASEL → ALU(“A”) → 存地址 → 数据经 WDSEL 写 Rc。

## 24. 异常（Exceptions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/968c0977554281e59c6eda210ce70695_Slide25.png" alt="Exceptions" width="80%"/>

指令无法执行时（非法 opcode / illop、地址越界、除零等）：停止用户程序，转入处理程序——可转储状态、或用软件模拟未实现指令再恢复。外部 I/O 事件则需**中断**当前程序、处理后无感恢复。硬件把异常当成强制过程调用，保存 PC+4 以便返回。这是用户程序与 OS 接口的关键。

## 25. 异常处理（Exception Processing）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/59e5c431cda8dc8a89492d7c78d99673_Slide26.png" alt="Exception Processing" width="80%"/>

打断当前程序，如同当前指令变成对 handler 的调用；handler 可用普通过程返回恢复。**Exception**：由当前程序某指令引起的同步异常。**Interrupt**：与程序无关的异步外部事件。

## 26. 异常实现（Exception Implementation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8fbc7c3a1729add37ce2ec5d276d399e_Slide27.png" alt="Exception Implementation" width="80%"/>

两类实现相同：硬件表现得像 taken BR 到 0x4（同步）或 0x8（异步）；PC+4 写入 **R30 = XP**（exception pointer）。用户程序不可用 XP（随时可能被中断覆盖）。例：未实现 DIV → illop → 0x4 → handler 用 XP 取非法指令并模拟，再 `JMP(XP)` 继续。

## 27. 异常 I（Exceptions I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6ec6428816eb7a3bd9779f6b59d89da5_Slide28.png" alt="Exceptions I" width="80%"/>

**WASEL** MUX：1 时写回地址强制为 XP（R30），0 时用 Rc。PCSEL 增加常量输入 0x4 / 0x8。

## 28. 异常 II（Exceptions II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2afa8d77a1c028852d34e03bfe57c108_Slide29.png" alt="Exceptions II" width="80%"/>

PC+4 经 WDSEL 写入 XP；PCSEL=3 或 4 进 handler。被打断指令**未执行**；若要重试须先对 XP 减 4 再 `JMP(XP)`。

## 29. Beta：最终答案（Beta: Our Final Answer）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5ec963163138f9e5dcd8316b83d7cf0d_Slide30.png" alt="Beta Our Final Answer" width="80%"/>

完整单周期数据通路已齐。硬件量不大，适合实验课亲手完成。现代 CPU 另有流水、多发射、复杂存储层次等（后续讲）。Beta 约 1–2 mm²，现代 Intel 芯片 300–600 mm²——多出来的面积为性能服务。

## 30. 控制逻辑（Control Logic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/faf09afca0fc5062e2a19e39c8ba3415_Slide31.png" alt="Control Logic" width="80%"/>

按指令类汇总控制表（含异常与 RESET）；无关信号标 don't care。MWE 与（多数情况下）WERF 必须有定义值。最简：opcode 索引 ROM；Z 与 IRQ 用少量逻辑修正 ROM 输出。亦可 Karnaugh 图做门级最小化——建议先 ROM 跑通再优化。

## 31. Beta Inside!

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c46606980a81ef685afb6415bd989aaf_Slide32.png" alt="Beta Inside" width="80%"/>

简单编码 + 只做常见操作的硬件；复杂/少见功能交给软件；异常机制在硬件不够时把控制交给软件。完成 Beta 设计是许多 MIT 学生的 “Yes!” 时刻——并收获 “Beta Inside” 贴纸。
