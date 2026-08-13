---
title: MIT 6.004：L12 过程与栈
date: 2026-08-11 10:12:00
categories: ic
tags:
  - ic
  - digital-circuit
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L12 注解幻灯片。
>
> 源网页：[12.1 Annotated Slides | Procedures and Stacks](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c12/c12s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L12：过程与栈（Procedures and Stacks）

本讲讲清过程（procedure）抽象为何需要**激活记录**（activation record），以及为何 LIFO **栈**能自然支持嵌套与递归；随后给出 Beta 上的 SP/LP/BP 约定、栈帧布局、callee-saves 契约，并用阶乘与“栈侦探”题目串起整套调用约定。

## 1. 过程：软件抽象（Procedures: A Software Abstraction）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/081188ef0c971ba238cf6ded49cc0a7d_Slide02.png" alt="Procedures A Software Abstraction" width="80%"/>

过程/子程序：完成特定任务的指令序列，有唯一命名入口。可有形式参数（formal parameters）；调用时实参（arguments）与之对应。过程体可定义仅在执行期间存在的**局部变量**；可返回值，也可只为副作用而执行。

例：`COPRIMES` 只需知道 `GCD` 的入参/返回类型即可调用——实现被封装成黑盒。标准库提供大量预建过程（数学、容器、文件等）。过程抽象是高级语言表达力的核心。

## 2. 实现过程（Implementing Procedures）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6a84e14f318c22650c320f431c84f184_Slide03.png" alt="Implementing Procedures" width="80%"/>

两种思路：

1. **内联**（inline）：用过程体副本替换调用，实参代入形参（类似宏）。短过程有时值得；长过程多次调用会膨胀代码。递归在编译期无法终止展开 → 内联失败。
2. **链接**（link）：过程代码只保留一份；调用方求实参、存约定位置，`BR` 到入口并保存**返回地址**（return address）；过程算完把结果放约定位置，再 `JMP` 回返回地址。

## 3. 过程调用约定（Procedure Calling Convention）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e8368b14ef6b223abc134f1abf2ee017_Slide04.png" alt="Procedure Calling Convention" width="80%"/>

需约定：实参放哪、返回值放哪。可尝试用寄存器：如 R1 起传参、R28 存返回地址（**linkage pointer**）、R0 返回值。`BR`/`JMP` 正好对应 call/return。

目标：所有调用与过程体用同一约定——含递归 `fact(n-1)` 与顶层 `fact(3)`。

## 4. 过程链接：初试（Procedure Linkage: First Try）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ef241e01bd813c73246098ac1b13fa15_Slide05.png" alt="Procedure Linkage First Try" width="80%"/>

按上述约定编译 `fact()`：入参进 R1，`BR` 保存返回地址到 R28；递归时又把 `n-1` 写入 R1、新返回地址写入 R28——**覆盖**外层还需要的 `n` 与返回地址。深度为 $d$ 时需要约 $2d$ 个存储槽，寄存器不够。禁止递归可绕过，但语言通常允许递归 → 必须动态为每次活跃调用分配存储。

## 5. 过程的存储需求（Procedure Storage Needs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a0d9b41751d20e538b786320403c9202_Slide06.png" alt="Procedure Storage Needs" width="80%"/>

每次活跃调用需要：实参、返回地址、返回值、局部变量、以及调用方寄存器现场。不想限制参数/局部变量个数 → 每次调用一块存储，即**激活记录**。不能静态只为某过程留一块（递归有多份同时活跃）→ 调用时分配、返回时回收。

## 6. 激活记录（Activation Records）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e8f40467996e7889aca7f27091a7ee21_Slide07.png" alt="Activation Records" width="80%"/>

`fact(3)`→`fact(2)`→`fact(1)`→`fact(0)`：每层调用创建自己的激活记录；嵌套调用返回后其记录先丢弃。LIFO：被调方记录总在调用方之前释放——调用方必须等被调返回才能结束。需要高效支持这种分配/释放的结构。

## 7. 洞察：需要栈！（Insight: We Need a Stack!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3f3a061f766688454036eb06a2074f3c_Slide08.png" alt="Insight We Need a Stack" width="80%"/>

激活记录按 LIFO 进出 → **栈**（stack）：`PUSH` 加顶、`POP` 去顶。C 过程通常只访问栈顶记录；Java 等可访问其他活跃帧。闭包 / continuation（如 Python `yield`）需在返回后仍保留帧 → 纯栈不够，属更高级课程话题。

## 8. 栈实现（Stack Implementation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7b5f6f95568e4daa6939dfe7bd74a57f_Slide09.png" alt="Stack Implementation" width="80%"/>

Beta：专用 **R29 = SP**（stack pointer）。`PUSH` 时递增 SP，栈向高地址生长。约定：SP 指向**下一个未用**字；低于 SP 的地址是已分配内容。

**栈纪律**（stack discipline）：一段代码 `PUSH` 的内容必须在结束前 `POP` 掉，使 SP 恢复原值。栈放在大块内存区，系统常设最大栈长，溢出报错。Beta 用现有指令实现栈，纯软件约定；有的 ISA 有专用栈指令。

## 9. 栈管理宏（Stack Management Macros）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/374ddee69cd65875183554a38608aa5d_Slide10.png" alt="Stack Management Macros" width="80%"/>

UASM 宏：

- `PUSH`：先 `ADDC` 分配栈顶，再 `ST` 初始化（顺序重要：中断可能夹在两指令之间；先分配再写更安全）
- `POP`：先 `LD` 取值，再 `SUBC` 释放
- `ALLOCATE` / `DEALLOCATE`：预留/释放 $N$ 个字（不必初始化）

见 `PUSH`/`ALLOCATE` 应能找到配对的 `POP`/`DEALLOCATE`。

## 10. 玩转栈（Fun With Stacks）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c4ede911868c94e358b88a4dc9b0a43b_Slide11.png" alt="Fun With Stacks" width="80%"/>

临时占用寄存器前可 `PUSH` 旧值，用完按相反顺序 `POP` 恢复（LIFO）。有了栈，即可解决激活记录的分配/释放问题。

## 11. 解决过程链接问题（Solving Procedure Linkage Problems）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/85312ef5c5bdb8f889237146690ae312_Slide12.png" alt="Solving Procedure Linkage Problems" width="80%"/>

栈上放激活记录：实参、局部变量、以及保存的 LP（以便嵌套调用）。职责分摊：

- **调用方（caller）**：求实参并**逆序** `PUSH`（第一实参最后压栈）；`BR` 到入口，返回地址进 LP=R28；返回后 `DEALLOCATE` 去掉实参。
- 被调方完成帧的其余部分（下页）。

逆序原因后文说明（可变参数）。

## 12. 栈帧作激活记录（Stack Frames as Activation Records）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1e894bddaceae105583a4892749d98fc_Slide13.png" alt="Stack Frames as Activation Records" width="80%"/>

入口序列完成帧（也称 **stack frame**）：保存 LP；再保存调用方的 **BP=R27**，并用当前 SP 设本帧 BP；`ALLOCATE` 局部变量；再 `PUSH` 本过程将破坏的寄存器（**callee saves**：除 R0 外寄存器跨调用保持）。

有了 BP，帧内访问用相对 BP 的固定偏移，比相对 SP 更稳（SP 会随 PUSH/POP 变）。

## 13. 栈帧细节（Stack Frame Details）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4aeaa59687e58fda07a9f0fc046e7264_Slide14.png" alt="Stack Frame Details" width="80%"/>

回到问题：为何实参**逆序**压栈？

## 14. 实参顺序与 BP（Argument Order & BP Usage）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9242020b05e48fd662c3c4127ac8e13e_Slide15.png" alt="Argument Order and BP Usage" width="80%"/>

逆序后，第一实参（arg 0）相对 BP 偏移固定（如 $-12$），与共压几个实参无关；第二参 $-16$…… C 的可变参数（如 `printf`：格式串在第一个实参）依赖这一点。局部变量亦固定偏移：第一局部 $0$，第二 $+4$…… 帧上更高地址可用于嵌套调用建新帧。

## 15. 过程链接契约（Procedure Linkage: The Contract）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/49fb4e5d5e1d900a34a02445f27bde2a_Slide16.png" alt="Procedure Linkage The Contract" width="80%"/>

**Caller**：逆序压实参 → `BR` 到入口（返回地址→LP）→ 返回后卸掉实参。

**Callee**：计算结果放 R0 → `JMP` 返回地址 → 卸掉自己压上的内容，栈恢复入口时模样 → 除 R0 外保持所有寄存器。

## 16. 过程链接模板（Procedure Linkage Templates）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3728697745d5f935b536dde97faeec40_Slide17.png" alt="Procedure Linkage Templates" width="80%"/>

入口：保存 LP、BP；`MOVE(SP,BP)`；`ALLOCATE` 局部；`PUSH` 将用寄存器。出口：按相反顺序 `POP` 恢复；`MOVE(BP,SP)` 隐式撤销 `ALLOCATE`；最后 `JMP(LP)` 回调用方。

## 17. 综合：阶乘（Putting It All Together: Factorial）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1d68a1469af72f47a3a0d7def3bebbc7_Slide18.png" alt="Putting It All Together Factorial" width="80%"/>

C 版 `fact` 的编译结果：入口/出口序列清晰；嵌套调用栈上传参并在返回后 `DEALLOCATE`；体用上讲模板。链接开销约十余条指令；对极短非递归过程，优化器可能选择内联。

## 18. 递归？（Recursion?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d86e5032f8ec023288fddaef8f2f7fad_Slide19.png" alt="Recursion" width="80%"/>

已解决：每次调用新栈帧；返回按逆序释放。沿 BP 链可读出活跃调用、实参、局部变量 → **stack trace**（运行时出错时常打印）。能解读栈帧 ≈ 理解调用约定（测验常见题）。

## 19. 栈侦探（Stack Detective）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d093fadfb09cb864306359f4572f6c87_Slide20.png" alt="Stack Detective" width="80%"/>

练习：已知 PC=0x40、栈转储，推断：

- 当前 `fact` 实参为 3；沿保存的 BP/LP 找到原始调用实参为 6
- 原始调用的 `BR` 在 0x7C（保存的 LP=0x80 为下一指令）
- 即将执行的是 `DEALLOCATE(1)`（递归返回后的指令）
- BP 等可由帧布局与十六进制地址推出

关键：会读帧布局、十六进制算术、区分递归返回地址与外部返回地址。

## 20. 专用寄存器小结（Summary of Dedicated Registers）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a65ff319444a8e484792c1b5dd6fddfa_Slide21.png" alt="Summary of Dedicated Registers" width="80%"/>

| 寄存器 | 角色 |
|--------|------|
| R31 | ISA 规定恒为 0 |
| R30 | 留给下讲 Beta 实现（XP）；用户代码勿用 |
| R29 (SP) | 栈指针 |
| R28 (LP) | 链接指针（返回地址） |
| R27 (BP) | 基址指针（当前帧） |

## 21. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/019cee61090a93d9f856a5cfbaba9f1a_Slide22.png" alt="Summary" width="80%"/>

激活记录由 caller/callee 共同建造，返回时丢弃；含实参、保存的 LP/BP、其他被保存寄存器、局部变量。BP 指向当前帧；**callee saves**（除 R0）保证嵌套与递归安全。至此可编译并执行任意 C 程序（在约定意义下）。
