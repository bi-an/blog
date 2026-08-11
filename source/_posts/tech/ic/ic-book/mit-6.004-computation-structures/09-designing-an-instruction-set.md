---
title: MIT 6.004：L09 指令集设计
date: 2026-08-11 17:32:00
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L09 注解幻灯片。
>
> 源网页：[9.1 Annotated Slides | Designing an Instruction Set](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c9/c9s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L09：指令集设计（Designing an Instruction Set）

从专用阶乘硬件出发，抽象出可编程 datapath 与控制 FSM，引入 von Neumann 存储程序模型，并定量设计本课的 **Beta** RISC ISA：寄存器、定长指令、ALU/访存/分支。

## 1. 例子：阶乘 I（Example: Factorial I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/78edbda355b4e46eb8025b1e782e02b6_Slide02.png" alt="Example: Factorial I" width="80%"/>

$N!=N\cdot(N-1)\cdots 1$。用 C 描述：变量 `a` 累乘结果，`b` 为下一乘数（初值 $N$）；循环中 `a=a*b`，`b=b-1`。

## 2. 例子：阶乘 II（Example: Factorial II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/30a64f22ab90270fce261a53d1fdd493_Slide03.png" alt="Example: Factorial II" width="80%"/>

用**高层 FSM** 描述：各态“输出”是对变量的运算公式，而非简单电平。状态序列对应 C 程序步骤；`b` 新值为 0 时进入 DONE。

实现：32 bit 寄存器存 `a`/`b`，2 bit 状态寄存器；逻辑判断 `b==0`，并实现乘、减一与选通写入。

## 3. 阶乘的 Datapath（Datapath for Factorial）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ca3abf969b196c3f15127760919756f1_Slide04.png" alt="Datapath for Factorial" width="80%"/>

**Datapath**：存值寄存器 + 组合运算。START：`a←1`，`b←N`；LOOP：`a←a*b`，`b←b-1`；DONE：保持。MUX（WASEL/WBSEL）选择写入值。

## 4. 阶乘的控制 FSM（Control FSM for Factorial）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2bce59951de75309b69a626d117e2ffb_Slide05.png" alt="Control FSM for Factorial" width="80%"/>

Datapath 给出 Z（新 `b` 是否为 0）。控制 FSM：输入 Z，输出 WASEL/WBSEL；真值表含当前态 $S$ 与下一态 $S'$。

## 5. 控制 FSM 硬件（Control FSM Hardware）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6ba0f97ff5175b98b3668d7e5af1d26a_Slide06.png" alt="Control FSM Hardware" width="80%"/>

乘/减一用组合电路；控制用寄存器+ROM：$Z$+2 bit 状态 → 3 输入，$2^3=8$ 单元，每单元 6 bit（WASEL、WBSEL、下一态各 2 bit）。

## 6. 目前：专用硬件（So Far: Single-Purpose Hardware）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/60942d9567f7c1d623d9c33daae661a8_Slide07.png" alt="So Far: Single-Purpose Hardware" width="80%"/>

流程：高层 FSM → datapath → 控制 FSM。整体也是 FSM，但 datapath 寄存器也算“状态”则约 66 bit → $2^{66}$ 行真值表不可行 → **datapath 与控制 FSM 分离思考**。

通用化：更多存储、更丰富运算集（最小充分集出人意料地小；复杂运算常拆成加减乘序列）。架构乐趣在折衷。

## 7. 简单可编程 Datapath（A Simple Programmable Datapath）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0540a7336a69e92b50f8dc688caafd25_Slide08.png" alt="A Simple Programmable Datapath" width="80%"/>

4 个数据寄存器；ASEL/BSEL 选操作数；OPSEL 选运算结果；WEN+WSEL 写回（寄存器带 load-enable）。控制 FSM 序列产生控制信号；Z 支持数据相关转移。

## 8. 阶乘的控制 FSM（A Control FSM for Factorial）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bf65ee84688b613160d96e4c9563d02b_Slide09.png" alt="A Control FSM for Factorial" width="80%"/>

通用 datapath 每拍一运算 → 每轮循环需乘、减一、判零三态。通用机往往比专用电路**更多周期、可能更多硬件**。

## 9. 新问题 → 新控制 FSM（New Problem → New Control FSM）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8e97c8a926efe7b772ec8969e150fb95_Slide10.png" alt="New Problem New Control FSM" width="80%"/>

同一硬件可做幂、除、开方等（寄存器 ≤4）。设计控制 FSM ≈ **编程**：规定运算序列。

## 10. ENIAC 计算机（The ENIAC Computer）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/549641b72f7e4e51232507454c29bb50_Slide11.png" alt="The ENIAC Computer" width="80%"/>

早期数字计算机正是如此工作。图为 1943 宾大 ENIAC。

## 11. 给 ENIAC 编程（Programming The ENIAC）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/49a5dee98d5a68e47025289ff4b41103_Slide12.png" alt="Programming The ENIAC" width="80%"/>

ENIAC 支持循环、分支、子程序，但映射到机器常需数周；拨开关、插线需数日，再单步调试。急需更轻便的编程方式。

## 12. von Neumann 模型（The von Neumann Model）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d9c1c2af7294972e4bab04d5253b1560_Slide13.png" alt="The von Neumann Model" width="80%"/>

现代机多基于 1945 von Neumann **存储程序**模型，三部分：

1. **CPU**：datapath + 控制 FSM
2. **主存**：约 $W$ 个 $N$ bit 字；CPU 发**地址**读写（延迟约数十 ns）
3. **I/O**：与外界通信、非易失存储等

## 13. 关键思想：存储程序计算机（Key Idea: Stored-Program Computer）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d57a2325f20ea5d5aa4814e2136a5cb8_Slide14.png" alt="Key Idea: Stored-Program Computer" width="80%"/>

指令与数据同存主存，皆为二进制。指令含 opcode、源/目的寄存器等字段；CPU 解释并执行，再取下一条。

如何区分指令与数据？看值本身不行——看**用法**：进 datapath → 数据；被控制逻辑取用 → 指令。

## 14. von Neumann 机解剖（Anatomy of a von Neumann Computer）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ec8d1a0caa4d695cb664932aa0c94834_Slide15.png" alt="Anatomy of a von Neumann Computer" width="80%"/>

- **Datapath（肌肉）**：寄存器、ALU、访存通路
- **控制单元（大脑）**：从主存取指令，译成 ASEL/BSEL/DEST/FN 等；含 **PC（program counter）** 指向下一条；接收状态以支持条件执行

32 个寄存器 → 选择信号各 5 bit；ALU 功能码可 6 bit。

## 15. 指令（Instructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6c3e02341c71f25e260275b0d8a7e3c0_Slide16.png" alt="Instructions" width="80%"/>

指令是基本工作单元。执行循环：按 PC 取指 → 译码控制 datapath → ALU 运算写回 → **PC ← 下一指令地址**。现代机能每秒数十亿条。

## 16. 指令集架构 ISA（Instruction Set Architecture）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dc61d42539d5fbe625191955db78e953_Slide17.png" alt="Instruction Set Architecture" width="80%"/>

**ISA** = 指令字段含义 + 存储/运算的功能规格，是硬件与程序员的契约。可换软件；硬件可升级而软件不变（x86 从约 30 万 IPS 到约 50 亿 IPS）。

警告：ISA 中嵌入的技术约束（寄存器数、字宽、地址空间）成功后难改——旧软件要跑在新机器上，坏选择可能背几十年。

## 17. ISA 设计（ISA Design）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/01b2712d08a818610771c410e6612fe5_Slide18.png" alt="ISA Design" width="80%"/>

难题：支持哪些运算？多少寄存器？多大内存？编码偏紧凑还是译码简单？

**定量方法**：选代表性 benchmark → 用拟议 ISA 实现并模拟 → 按速度/能耗/面积/成本评估。原则：**识别常见操作并优化它们**（通用计算中算术与访存极常见）。本课机器称 **Beta**。

## 18. Beta ISA：存储（Beta ISA: Storage）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/aa9abe6b2bde90c23f77f04833a8932e_Slide19.png" alt="Beta ISA: Storage" width="80%"/>

Beta 是 **RISC**：多数指令只访问内部寄存器；访存用独立 LD/ST，地址计算简单。同类：ARM、MIPS；x86 更复杂。

CPU 状态：32 bit **PC**；**32 个 32 bit 寄存器** R0–R31（指令中 5 bit 编号）。**R31 恒为 0，写入无效**。主存为 32 bit 字（4 字节），仅支持字访问，但用**字节地址**：相邻字地址差 4。常用 `0x` 十六进制。

## 19. 存储约定（Storage Conventions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/15e84db13f5631eddf569c9a742f0419_Slide20.png" alt="Storage Conventions" width="80%"/>

变量住在主存固定地址。算 `y=x*37`：LD x→R0，乘 37，ST 回 y。热数据尽量留在寄存器。模板：**load → 计算 → store** → **load-store 架构**。

## 20. Beta ISA：指令（Beta ISA: Instructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4a66d150bbb400c86f1eeb9797f45aac_Slide21.png" alt="Beta ISA: Instructions" width="80%"/>

三类：计算、LD/ST、分支。全部 **32 bit 定长**，占一字 → 译码简单；多数指令下一地址 = PC+4。

定长常不如变长紧凑，但变长译码复杂、能耗/性能代价高；当今内存相对充裕，Beta 选择定长以换小而快的执行引擎。

## 21. Beta ALU 指令（Beta ALU Instructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/136f068be8fca7f8df21f14b9ef4d539_Slide22.png" alt="Beta ALU Instructions" width="80%"/>

字段：6 bit **opcode**，5 bit **ra/rb** 源，5 bit **rc** 目的；其余填 0。Opcode 固定在 [31:26]。

例：ADD，opcode `0b100000`，`ADD(r1,r2,r3)` → R3←R1+R2；编码 `0x80611000`。同一寄存器可兼源与目的（如 R1←R1+R1 即 ×2）。助记符比二进制好写。

## 22. 实现草图 #1（Implementation Sketch #1）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ab15da9c0e9f3c3faf009d5be579b604_Slide23.png" alt="Implementation Sketch #1" width="80%"/>

ra/rb 选操作数（R31→常数 0）；rc 选写回。Opcode→ALU 功能可用 64 项 ROM。PC 每指令 +4。RISC 好处：许多字段可直接作控制信号。

## 23. 是否支持常数操作数？（Should We Support Constant Operands?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0c7ab2a57a182c554b1e7bd0657f9d49_Slide24.png" alt="Should We Support Constant Operands" width="80%"/>

用定量法评估“第二操作数可为小常数”：腾出 16 bit 常数域。看**实际执行**（非静态出现次数）：算术指令过半第二操作数为小常数；比较约 80%；地址计算亦常见 → **批准该特性**（程序更小更快）。

## 24. 带常数的 Beta ALU 指令（Beta ALU Instructions with Constant）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/16a01bf2e05ba7990874b729730d1abd_Slide25.png" alt="Beta ALU Instructions with Constant" width="80%"/>

第二格式：rb 换为 16 bit 补码常数（$-32768\sim 32767$）。例：`ADDC(r1,-3,r3)`。16→32 bit 用**符号扩展**（复制符号位），纯布线即可。助记符加后缀 **C**；超 16 bit 常数须放主存再 LD。

## 25. 实现草图 #2（Implementation Sketch #2）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3210daba87d1c166f648bde306e64a72_Slide26.png" alt="Implementation Sketch #2" width="80%"/>

多一个 MUX：BSEL=1 选符号扩展常数，否则选 rb。细节后几讲再展开。

## 26. Beta 的 Load / Store（Beta Load and Store Instructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9d61c14cbcc83fbd2684c2d44dc37cd2_Slide27.png" alt="Beta Load and Store Instructions" width="80%"/>

访存**唯一**途径。地址 = Ra + 符号扩展(const)（与 ADDC 同硬件）。LD：内存→Rc；ST：Rc→内存。ST 特殊：唯一需读 Rc；符号形式中 Rc 写在前；唯一**不写寄存器堆**的指令。

## 27. 使用 LD 与 ST（Using LD and ST）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/596728265d7364130f37f263d9b98484_Slide28.png" alt="Using LD and ST" width="80%"/>

`y=x*37` 三条：`LD(0x1008,r31,r?)` 等。地址适合 16 bit 常数时直接编入；更大地址当大常数存内存。低内存放不下全部大常数时，可用后讲的 **LDR（load relative）**。

## 28. 仅用 ALU 能解阶乘吗？（Can We Solve Factorial with ALU Instructions?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5c690db57723ecdab177dddf9bd0cd23_Slide29.png" alt="Can We Solve Factorial with ALU Instructions" width="80%"/>

顺序执行不够：需循环/跳过 → **条件分支**。条件成立则 PC 改到 **branch target**；否则 PC+4。用于循环、if、过程调用等。

## 29. Beta 分支指令（Beta Branch Instructions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ba9ca7a113a57a6f19551bceb45ae7f4_Slide30.png" alt="Beta Branch Instructions" width="80%"/>

**BEQ**：先把 PC+4 写入 Rc（不需要可写 R31）；若 Ra==0 则 PC 加 **字偏移×4**（PC-relative）；否则 PC+4。偏移 0 = 下一条；$-1$ = 分支自身。负偏移常用于循环回跳；正偏移用于 if 前跳。

`BEQ(R31,…)` 恒成立 → 无条件分支。**BNE**：Ra≠0 时跳转。

## 30. 现在能写阶乘了吗？（Can We Solve Factorial Now?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c1f07d0b0222586238fa858c2709e19c_Slide31.png" alt="Can We Solve Factorial Now" width="80%"/>

迭代阶乘：标号 `L:` 处 MUL、减一，`BNE` 回 `L`。符号形式写标号，由汇编器算偏移。与早先高层 FSM 各态高度对应（datapath 相似时常见）。

## 31. Beta JMP 指令（Beta JMP Instruction）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f33c4794a89597a3495d9aaeaa1eabf1_Slide32.png" alt="Beta JMP Instruction" width="80%"/>

**JMP**：PC←Ra，并可选保存 PC+4 到 Rc。配合无条件分支：调用时 BEQ 跳到过程并保存返回地址；过程末 JMP 返回。同一过程可从多处调用（返回地址不同）。过程细节后讲。

## 32. Beta ISA 小结（Beta ISA Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/66856949cb3c8902b07464d0bce19aba_Slide33.png" alt="Beta ISA Summary" width="80%"/>

- 32 个寄存器；程序与数据在主存；$2^{32}$ 字节地址空间；字访问、地址为 4 的倍数
- 两种格式：opcode+三寄存器，或 opcode+两寄存器+符号扩展 16 bit 常数
- 三类指令：ALU、LD/ST、分支与 JMP

下一讲：用这套简单运算表达任意可计算过程。
