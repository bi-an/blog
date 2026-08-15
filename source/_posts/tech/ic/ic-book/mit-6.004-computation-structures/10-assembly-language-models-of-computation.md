---
title: MIT 6.004：L10 汇编语言与计算模型
date: 2026-08-11 10:10:00
categories: ic
tags:
  - ic
  - digital-circuit
  - assembly
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L10 注解幻灯片。
>
> 源网页：[10.1 Annotated Slides | Assembly Language, Models of Computation](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c10/c10s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L10：汇编语言与计算模型（Assembly Language, Models of Computation）

本讲介绍 Beta 的 **UASM** 汇编（符号、标号、宏、伪指令），再上升到计算模型：FSM 局限、图灵机、Church 论题、通用机与不可计算性（停机问题），说明 Beta ISA 的图灵完备性。

## 1. Beta ISA 回顾（Beta ISA Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9e9d921dc3a0f185b7cec60c0d37525a_Slide02.png" alt="Beta ISA Summary" width="70%"/>

回顾：32 个通用寄存器 + PC；主存最多 $2^{32}$ 字节（$2^{30}$ 个 32 bit 字），指令与数据同存。指令 32 bit：6 bit OPCODE，5 bit Ra/Rb/Rc；两种格式（三寄存器 / 两寄存器+16 bit 常数）。三类：ALU、Load/Store、分支与跳转。

## 2. 编程语言（Programming Languages）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c0a9e5b7818c1d14c9a1f7e305eeb81d_Slide03.png" alt="Programming Languages" width="70%"/>

手写二进制编码不现实 → **汇编**用符号写 opcode 与操作数；仍需管寄存器与指令序列。**高级语言**再升一层：变量与数学运算。本讲 UASM；下讲 C→汇编。还可再叠解释器（如 C 实现 Python）——选合适语言表达，经多层翻译落到 Beta 指令。

## 3. 汇编语言（Assembly Language）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/803258193101fa7ce91495c7d30f09b0_Slide04.png" alt="Assembly Language" width="70%"/>

**汇编器**读文本 → 产出初始化主存的 32 bit 字数组。BSim 内建 **UASM**：实质是花哨计算器——求值算术表达式得字节，依次填入字节数组。支持符号/标号命名值与地址，宏封装指令/数据的字段拼装。

## 4. UASM 源文件例子（Example UASM Source File）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/eccce1b7cf0085d41432db93e6f56524_Slide05.png" alt="Example UASM Source File" width="70%"/>

通常一行一条语句。注释：`//` 至行末，或 `/* … */` 跨行。

- **符号（symbol）**：常量的名字，如 `N=12`；改一处即可。R0–R31 预定义为 0–31，便于区分寄存器与立即数
- **标号（label）**：某内存地址的名字（如下文 `loop`）

## 5. 如何汇编？（How Does It Get Assembled?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/098b9b899d278695ac1f1632d6cde100_Slide06.png" alt="How Does It Get Assembled" width="70%"/>

维护**符号表**（初含寄存器符号）。逐行：定义符号/标号、展开宏、求值写字节。例：`N=12` 入表；`ADDC(r31,N,r1)` 展开为地址 0 的 32 bit 字；`loop:` 记下当前地址后展开 MUL。

**两遍扫描**：第一遍收齐符号/标号；第二遍生成二进制 → 支持前向引用（如向前分支）。

## 6. 寄存器是预定义符号（Registers Are Predefined Symbols）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c34de41b21bc8ff905a222d5e7cd4a0f_Slide07.png" alt="Registers Are Predefined Symbols" width="70%"/>

寄存器无魔法：只是 0–31 的符号。`ADDC(r31,N,r1)` 实际变成 `ADDC(31,12,1)`。若把寄存器符号用在期望立即数处（或反之），UASM 仍按数值解释——**操作数含义由 opcode 宏决定，不由写法直觉决定**，写汇编须清醒。

## 7. 标号与偏移（Labels and Offsets）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ae8ce5295a2799e81f3a5866ea4ee9f3_Slide08.png" alt="Labels and Offsets" width="70%"/>

分支用相对**下一条指令**的字偏移（$-1$ 指向分支自身）。宏内嵌偏移公式：程序员写目标标号，UASM 算 16 bit 补码。例：BNE 回跳 3 条 → 偏移 $-3$。

## 8. 强大的宏指令（Mighty Macroinstructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2f7b5a04d7c97e3d25610f5dbc905867_Slide09.png" alt="Mighty Macroinstructions" width="70%"/>

宏：参数替换后当原文再处理，可嵌套。`WORD`/`LONG` 把值拆成 2/4 字节。Beta 为 **little-endian**：最低有效字节在最低地址（如 `0xDEADBEEF` 在 `0x100` 处先存 `0xEF`）。亦有 big-endian；跨 ISA 传多字节值常需转换。名称源自《格列佛游记》大小端之争。

## 9. 指令的汇编（Assembly of Instructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/05d247e1135be2a33849801c6b34b8a0_Slide10.png" alt="Assembly of Instructions" width="70%"/>

辅助宏 `BETAOP`：三寄存器格式；`.align 4` 保证字对齐；`LONG` 拼字段：各域 `%` 截断再 `<<` 到位。`BETAOPC` 处理含 16 bit 常数的格式。例：ADDC 把 opcode `0x30`、RA、常数、Rc 移位或运算拼成一字——“assemble” 字面义。

## 10. 汇编例子（Example Assembly）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f422757219fc2842a7d647b2bf5866cc_Slide11.png" alt="Example Assembly" width="70%"/>

一次 ADDC 的完整宏展开链：格式与 opcode 知识封在宏体里。换一套宏定义，UASM 可服务几乎任意 ISA。

## 11. Beta 指令的 UASM 宏（UASM Macros for Beta Instructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8516efbb264d4ad0d7997803c2d48680_Slide12.png" alt="UASM Macros for Beta Instructions" width="70%"/>

定义在 `beta.uasm`（实验会 include）。便利宏：分支常丢弃 PC+4 → 两参数形式默认 Rc=R31，少打字、更易读。

## 12. 伪指令（Pseudoinstructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/65e400955a1b5b4895381a58f06fc610_Slide13.png" alt="Pseudoinstructions" width="70%"/>

可读性宏：`BR()` 代替 `BEQ(R31,…)`；`BF`/`BT` 配合比较结果；`PUSH`/`POP` 展开为多指令操作 SP 栈。称 **pseudoinstructions**：表面上更大指令集，底层仍是 L09 那套。

## 13. 用伪指令写阶乘（Factorial with Pseudoinstructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/857deedf5de26daff99d1f03ecc9811e_Slide14.png" alt="Factorial with Pseudoinstructions" width="70%"/>

例：`CMOVE` 表示“装小常数”，比展开后的 `ADDC`（加到 0）更易懂。减少认知噪音长期受益。

## 14. 原始数据（Raw Data）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0b60eaa46fdf228d08013f5feb045bea_Slide15.png" alt="Raw Data" width="70%"/>

`LONG` 分配并初始化数据；标号记地址（如 N→0，factN→4）。`LD` 便利宏默认 Ra=R31：地址 = 0 + 常数（标号值）→ 把该处字装入寄存器。

## 15. UASM 表达式与布局（UASM Expressions and Layout）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/035604ee11225a633969f53fcbb57b2d_Slide16.png" alt="UASM Expressions and Layout" width="70%"/>

表达式在**汇编时**求值，不生成运行时 ADD/MUL。特殊符号 **`.`（dot）** = 下一个将填充的地址；初值 0，每写一字节递增。可赋 `.` 指定放置位置；`k:` 等价 `k=.`；也可增大 `.` 预留未初始化数组空间。

## 16. 小结：汇编语言（Summary: Assembly Language）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/613eedef9d85df82875095b9c774ac69_Slide17.png" alt="Summary: Assembly Language" width="70%"/>

汇编 = 方便生成指令/数据二进制并跟踪地址。UASM：值、符号、标号、宏、`.`。鸡生蛋：第一个汇编器靠**手工**汇编二进制，再逐步加符号/宏等特性——且务必备份二进制。

## 17. 通用性？（Universality?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e83fbfbf84ebbf1ca81bd369302a9383_Slide18.png" alt="Universality" width="70%"/>

NAND 对布尔函数通用。ISA 是否通用？能解 FSM 能解的问题吗？有 FSM 不能解的吗？Beta 能否解？答案依赖计算的数学模型。

## 18. 计算模型（Models of Computation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1a65599ea4c3afa548260678f83ad3c3_Slide19.png" alt="Models of Computation" width="70%"/>

CS 根源之一：比较各模型能表示的计算类，寻找**通用模型**——凡其它良构模型能描述的，通用模型也能。候选：**FSM**（时序逻辑可建；可用布尔与转移图 100% 预测行为）。FSM 是否万能数字计算装置？

## 19. FSM 的局限（FSM Limitations）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a495352d18e5d088dee2facf98155a6c_Slide20.png" alt="FSM Limitations" width="70%"/>

括号匹配：判定括号串是否良构（每个开括号有对应闭括号）。FSM 用有限状态记历史——括号检查需计数未匹配开括号，但状态数有上限 → 输入开括号过多则无法正确判定。

**有限性**限制了需**无界计数**的问题。转向 Alan Turing 的模型。

## 20. 图灵机（Turing Machines）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e12185e24dfe7c3f9267fd7cf08f029d_Slide21.png" alt="Turing Machines" width="70%"/>

1930 年代 Turing 提出：FSM + **无限纸带**（可读写）。输入编码在带上；FSM 读写、改态、写答案后停机 → **图灵机（TM）**。

有限非空白输入可编码成大整数；TM 实现整数到整数的函数。FSM 真值表可枚举并赋索引 → 谈“TM 347 在输入 51 上得 42”。

## 21. 其它计算模型（Other Models of Computation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/71d314029f6449c597a42380c7a383c1_Slide22.png" alt="Other Models of Computation" width="70%"/>

Kleene、Post、Turing 等（Church 学生）探索递归函数、字符串重写、λ 演算等，并关注**不可实现机解决的问题**——亦即刻画可解决类。

## 22. 可计算性？（Computability?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fe8ca2961d2c0971aa4be57e6057a2f5_Slide23.png" alt="Computability" width="70%"/>

各模型能算的整数函数集相同（可互译）。**Church 论题**：凡可实现机可算的离散函数，皆可由某 TM 计算。尚无严格证明，但被普遍接受。“可计算”≈“某 TM 可计算”。不可计算函数见本讲可选视频。

## 23. 众多图灵机！（Turing Machines Galore!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/95a9079735c92bc2f872124cd1ed99e4_Slide24.png" alt="Turing Machines Galore" width="70%"/>

每种想做的计算对应一台（不同）TM。这对通用计算机设计有何启示？是否有些计算永远需要专用机？

## 24. 通用函数（The Universal Function）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6354befe84e3d03feae8ffdf6f3e66bd_Slide25.png" alt="The Universal Function" width="70%"/>

希望有 $U(k,j)=$ 运行 $T_k$ 于输入 $j$ 的结果。**$U$ 可计算**：存在通用图灵机 $T_U$（且有无穷多；已知最小者约 4 态、6 带符号）。一台通用机可完成任意 TM 能做的计算。

## 25. 通用性（Universality）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/45b7e7afe667521b9394434a076a3aed_Slide26.png" alt="Universality" width="70%"/>

$k$ 编码“程序”（某 TM 描述），$j$ 编码数据；$T_U$ **解释**程序、模拟 $T_k$。解释编码计算 = 存储程序计算机的核心思想。

## 26. 图灵通用性（Turing Universality）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1d35275217e5f639a6b93e67cedf1af2_Slide27.png" alt="Turing Universality" width="70%"/>

通用 TM 是现代通用计算机的范式。证明 ISA 图灵通用：展示能模拟某已知通用 TM。实际机器内存有限 → 仅对放得下的输入等价。门槛不高：**有条件分支 + 简单算术** 通常即足够。

## 27. 编码算法：CS 关键（Coded Algorithms: Key to CS）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fca78e4837f83e1311ef1233555c19e1_Slide28.png" alt="Coded Algorithms: Key to CS" width="70%"/>

程序可作为另一程序的数据 → **编译**（高级语言→汇编）、软件组件复用、设计面向任务的语言。结论：拟建引擎能做任意可实现机上的计算；通用 TM 为存储程序机铺路 → **Beta ISA 够用**。

## 28. 不可计算！（Uncomputability!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a8ad6ba1913bf75c84feb13ca9621039_Slide29.png" alt="Uncomputability" width="70%"/>

存在良定义离散函数，**无任何 TM** 能在有限步对任意有限输入算出 $f(x)$——可证明算法不存在。最著名：**停机函数**——给定 $(k,j)$，判定第 $k$ 个 TM 在输入 $j$ 上是停机还是永远循环。

## 29. 为何 $f_H$ 不可计算（Why fH is Uncomputable）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8f25fca4a51591b03276d60411d27f63_Slide30.png" alt="Why fH is Uncomputable" width="70%"/>

反证：若停机函数可算，则有 $T_H$。构造“nasty”机 $T_N$：$T_N[X]$ 在 $T_X[X]$ 停机时循环，在 $T_X[X]$ 循环时停机（靠 $T_H$ 查询）。再喂 $N$ 给 $T_N$：$T_N[N]$ 必须既停又不停 → 矛盾。故 $T_H$ 不存在，停机函数不可计算。
