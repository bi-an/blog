---
title: MIT 6.004：L11 编译器
date: 2026-08-11 10:11:00
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L11 注解幻灯片。
>
> 源网页：[11.1 Annotated Slides | Compilers](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c11/c11s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L11：编译器（Compilers）

本讲从高级语言出发，对比**解释**（interpretation）与**编译**（compilation），再用递归下降模板把 C 表达式/语句翻译成 Beta 汇编；随后拆解现代编译器前端（词法/语法/语义）、中间表示（IR / CFG）与多遍优化，最后做代码生成。

## 1. 编程语言（Programming Languages）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/34f32b11c581ff12667156e220c1d1ec_Slide02.png" alt="Programming Languages" width="80%"/>

已学 Beta ISA：对寄存器中的 32 位数据做 datapath 操作，并访存、改 PC；指令由 opcode / 源 / 目的等字段组成 32 位字。汇编一句对应一条指令，程序员自己管寄存器与主存，并把数组访问等拆成 Beta 操作序列。

高级语言用变量与数据结构抽象存储与搬移；用表达式与赋值 `=` 等紧凑描述本需大量汇编的计算。本讲核心：如何把高级语言程序翻译成可在 Beta 上运行的代码。

## 2. 高级语言（High-Level Languages）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/738299146f0f4ed47c7bcfff7b478be4_Slide03.png" alt="High-Level Languages" width="80%"/>

以欧几里得求 GCD 的 C 代码为例；课程用 C 的简单子集作示例语言。C 由 Dennis Ritchie 在 AT&T Bell Labs 为 Unix 而开发；此后语言不断加入 OOP、新数据结构与控制结构。

用高级语言可不提寄存器、具体指令等 ISA 细节 → 写得更快、更易读、更易维护；类型检查可拦下把字符串赋给数值变量等错误；动态分配等可自动化。因抽象掉具体 ISA，同一源码可移植到不同机器。代价取决于执行策略：**解释**还是**编译**。

## 3. 解释（Interpretation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/48f454c0102fb9926d436a2eff7218e9_Slide04.png" alt="Interpretation" width="80%"/>

在真实机器 M1 上跑**解释器**，模拟易编程的抽象机 M2：每条 M2 操作由一段 M1 指令序列实现。解释器 + M1 ≡ M2 的一种实现。

常见多层解释：笔记本 x86 跑 Python 解释器 → 加载 SciPy → 一条 SciPy 命令展开为大量 Python 语句 → 每条 Python 再变成数百条 x86。适合一次性计算或探索算法；不必手写全部机器指令。

## 4. 编译（Compilation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/29a385517f7edf981057120150adb734_Slide05.png" alt="Compilation" width="80%"/>

对需反复执行、愿付前期成本的任务用编译：把高级程序 P2 **逐语句翻译**成 M1 上的等价程序 P1（并不在翻译时“跑”P2）。做翻译的程序叫**编译器**（compiler）。

编译一次得 P1，之后直接跑 P1，避免运行时解析源码与多层解释开销。换不同编译器可把同一 P2 落到 M2、M3… 而不改写源码。解释与编译都能改源码、抽象真实机器，且在现代系统中并存。

## 5. 解释 vs. 编译（Interpretation vs. Compilation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/233447eb861533c3c448eb9ef9d4b661_Slide06.png" alt="Interpretation vs Compilation" width="80%"/>

对语句 `x+2`：解释器处理时立刻取 `x` 并加 2；编译器则生成 `LD`/`ADD` 等指令，留待以后执行。循环中解释器会反复处理同一语句；编译器只生成指令一次。

| | 解释 | 编译 |
|--|------|------|
| 开销时机 | 执行中反复处理源码 | 一次性编译，执行更快 |
| 类型/操作决策 | 运行时，更灵活 | 编译期，换速度 |
| 开发循环 | 改完即可跑 | compile–run–debug 可能更慢 |

一般编译代码快得多；解释器可按 `x` 的实际类型改变行为。

## 6. 编译器（Compilers）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/825798a692e25db597d14e2da4a6398c_Slide07.png" alt="Compilers" width="80%"/>

编译器：把高级语言程序翻译成功能等价的机器指令序列（汇编）。先检查良构性：语句是否合法、有无无意义的运算（字符串+整数）、未初始化就用等；并警告浮点转整型可能溢出等。

通过检查后生成高效指令，常重排计算使序列更短更快。现代优化编译器耐心探索替代方案，往往难被手写汇编全面超越。本节先看简单编译策略，再看现代编译器结构。

## 7. 简单编译策略（A Simple Compilation Strategy）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ebbf386f679fd739c66c430c39218887_Slide08.png" alt="A Simple Compilation Strategy" width="80%"/>

两主例程：`compile_statement` 与 `compile_expr`。源程序是语句序列，反复调前者。关注四类语句：无条件（求一次表达式）、复合（顺序执行子语句）、条件（`if`：测表达式真则执行 then）、以及迭代（后文 WHILE）。

`compile_expr`：生成求值表达式并把结果放在某寄存器的代码。表达式含常量、标量/数组变量、赋值、一元/二元运算、过程调用等。复杂算术可拆成一元/二元序列。过程调用留待下讲；其余表达式与语句可直接模板化。

## 8. compile_expr(expr) → Rx

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/db87d6c47aeb9561ee46c34daad70753_Slide09.png" alt="compile_expr expr to Rx" width="80%"/>

- **小常量**（$-32768\sim +32767$）：`CMOVE` 符号扩展进寄存器；过大则存主存再 `LD`。
- **变量**：与大常量类似，`LD` 对应地址。
- **数组访问**：元素连续存放；先 `compile_expr` 得下标于 Rx，再乘元素字节数 `bsize`（如 `int` 为 4）得字节偏移，`LD` 基址+偏移取元素。
- **赋值**：`compile_expr` 得右值，再 `ST` 到左值变量。
- **算术**：分别编译操作数到寄存器，再生成对应 ALU 指令。

## 9. 编译表达式（Compiling Expressions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fc46224972d36e38f05d11df9762adec_Slide10.png" alt="Compiling Expressions" width="80%"/>

例：含减、乘、加的赋值。按赋值模板递归编译 RHS；遇乘则再编译左操作数（减）……直至叶节点（变量/常量）生成 `LD`/`CMOVE`，再沿表达式树上返回生成 `SUB`/`MUL`/`ADD` 等。

此即**递归下降**（recursive descent）：每层表达式更简单，直到叶。相邻指令还可做**窥孔优化**（peephole）：如 `CMOVE` 后跟算术常可合并为带常量操作数的单条指令。

## 10. compile_statement

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9caa57defd4fffd7bed534f088dd1f91_Slide11.png" alt="compile_statement" width="80%"/>

- **无条件语句**：多为赋值或过程调用 → 交给 `compile_expr`。
- **复合语句**：对每个子语句递归 `compile_statement`；生成代码首尾相接，顺序执行。

## 11. compile_statement：条件（Conditional）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ba763f151457b1ac2dc01f44414ca27e_Slide12.png" alt="compile_statement Conditional" width="80%"/>

最简 `if`：编译 test；若寄存器为 FALSE，分支跳过 THEN 子句代码。完整 `if–else` 用分支与标签保证：真走 then，假走 else，最后汇合。

编译本质：套用许多小模板，逐步把代码生成拆成更小任务，并用分支把碎片粘成正确控制流。

## 12. compile_statement：迭代（Iteration）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d944798591b63162596bfdf8cb6d681b_Slide13.png" alt="compile_statement Iteration" width="80%"/>

`while` 模板类似 `if`，末尾再分支回测表达式，直到为假。可重组使每轮只需一条 `BT`（原模板每轮 `BF`+`BR`），循环内小优化可累积为大收益。

`for` 可改写成带更新的 `while`，再套上述模板。

## 13. 综合：阶乘（Putting It All Together: Factorial）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/45bfd0bcd813e7cbbe9323f931d7f1c0_Slide14.png" alt="Putting It All Together Factorial" width="80%"/>

把模板套到迭代版阶乘：生成代码可与前述模板一一对应。非最优，但递归下降已够用。

## 14. 优化：值留在寄存器（Optimization: keep values in regs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a941a67234d1f22d3fa42c7c702efb69_Slide15.png" alt="Optimization keep values in regs" width="80%"/>

让变量住在专用寄存器而非主存，可少掉大量 `LD`/`ST`。例：循环内指令从 10 条减到 4。优化编译器擅长找此类机会。下文改谈更一般的现代编译器流程。

## 15. 现代编译器解剖（Anatomy of a Modern Compiler）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e0df7a87249a55fe123034ed552b2041_Slide16.png" alt="Anatomy of a Modern Compiler" width="80%"/>

- **前端 / 分析**：检查语法与语义（类型等），把源程序变成机器无关的**中间表示**（IR）。多语言前端可共享同一 IR。
- **后端 / 综合**：先对 IR 做优化（如把与循环下标无关的运算提出循环），再为目标 ISA 生成指令，并做 ISA 相关窥孔优化（如 Beta 上 `CMOVE`+算术合并）。

## 16. 前端：词法分析（Frontend Stages: Lexical Analysis）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9b4c4190a20ac6643dfa4bc5f291932b_Slide17.png" alt="Frontend Stages Lexical Analysis" width="80%"/>

扫描源文本 → **token** 序列；空格/制表/换行仅作分隔，扫描后去掉。token 带文件名、行号、列号以便报错。非法 token（如 C 中 `3x`）在此阶段报错。

## 17. 前端：语法分析（Frontend Stages: Syntactic Analysis）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2aea3cfc8fef0a08bbb45daad2105917_Slide18.png" alt="Frontend Stages Syntactic Analysis" width="80%"/>

**解析**（parsing）把 token 建成**语法树**（syntax tree）：操作数挂到一元/二元结点，语句各部件标好角色。树结点标签与前述代码模板对应；深度优先遍历即可按标签选模板——但先还要做语义分析与变换。

## 18. 前端：语义分析（Frontend Stages: Semantic Analysis）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/64a9cb31fc7ff29454ea494731283989_Slide19.png" alt="Frontend Stages Semantic Analysis" width="80%"/>

在语法树上检查语义：如 `x = "bananas"` 语法合法（左变量、右表达式），但若 `x` 声明为 `int`、右为 string，则类型不兼容。查符号表比对类型。完成后：语法树表示语法正确且语义有效的、语言无关的操作序列。

## 19. 中间表示 IR（Intermediate Representation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/09e316d5be168697e77c1b9932ba9b88_Slide20.png" alt="Intermediate Representation IR" width="80%"/>

语法树是常用 IR：独立于源语言与目标 ISA；保留运算顺序与分组信息；允许多前端共用一后端。后端可再分：机无关 IR 优化 → 代码生成到目标 ISA。

## 20. 常用 IR：控制流图（Common IR: Control Flow Graph）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5283f7ef2c81e6613a47b1287b97112b_Slide21.png" alt="Common IR Control Flow Graph" width="80%"/>

把语法树重组为**控制流图**（CFG）：结点为**基本块**（basic block）——以分支结束的赋值/表达式序列；一旦进入块，块内其余操作会连着执行。边表示跳转到哪一块。基本块边界清晰，便于寄存器暂存变量等优化。

## 21. GCD 的控制流图（Control Flow Graph for GCD）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e2851f4acb66e276dd58f805c78547ff_Slide22.png" alt="Control Flow Graph for GCD" width="80%"/>

条件分支块有标 `T`/`F` 的两条出边；无条件则单出边。若某块仅一个前驱，可继承前驱关于寄存器中已有 `x`、`y` 等知识；多前驱时只能用**所有**前驱共有的知识。CFG 很像高级 FSM 的状态转移图。

## 22. IR 优化（IR Optimization）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9745dcee00abd38fedf6bdbf64c28ad6_Slide23.png" alt="IR Optimization" width="80%"/>

对 CFG 多遍简单优化，反复直到无进展；简单变换可叠加出复杂效果。例：

- **死代码消除**（dead code elimination）：删从未用的赋值、不可达块
- **常量传播**（constant propagation）：已知常量的变量用常量替换引用
- **常量折叠**（constant folding）：编译期求常量表达式

## 23. IR 优化示例 I（Example IR Optimizations I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c07ac52814a15709afe1089f313e91a4_Slide24.png" alt="Example IR Optimizations I" width="80%"/>

略造作的源程序及其 CFG：复杂表达式拆成简单二元运算，中间结果用临时名如 `_t1`。

## 24. IR 优化示例 II（Example IR Optimizations II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d5bb16690b95c1a92d8126b1593dc24a_Slide25.png" alt="Example IR Optimizations II" width="80%"/>

死代码消除去掉第一块中对 `Z` 的赋值（后续再赋且中间未用）；发现 `X=3` 且不再赋值 → 传播常量 3；再常量折叠。

## 25. IR 优化示例 II（续）（Example IR Optimizations II continued）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/aa454c55254f5781d15cc3a3d3ec2894_Slide26.png" alt="Example IR Optimizations II continued" width="80%"/>

更新后的 CFG 再一轮：死代码 → 常量传播 → 常量折叠。

## 26. IR 优化示例 III（Example IR Optimizations III）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1fb9ffb6c636f88ddc811b97c8d91b45_Slide27.png" alt="Example IR Optimizations III" width="80%"/>

两轮后赋值大减。第三轮：死代码；并可判定条件分支结果 → 删空块或不可达块。

## 27. IR 优化示例 IV（Example IR Optimizations IV）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c57a3560c024f3d97fc001d3247a0e63_Slide28.png" alt="Example IR Optimizations IV" width="80%"/>

IR 明显变小。继续常量传播、折叠、死代码消除。

## 28. IR 优化示例 IV（续）（Example IR Optimizations IV continued）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/70fbef84308277a4b8a8a22f4dac1221_Slide29.png" alt="Example IR Optimizations IV continued" width="80%"/>

直到无更多优化。简单变换反复应用，得到算同一最终 `Z` 的更小程序。还可加：公共子表达式消除、循环无关代码外提、短循环展开等。

## 29. 代码生成（Code Generation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/028d57410f7f34e71ef7d055a5c962bb_Slide30.png" alt="Code Generation" width="80%"/>

1. 为变量分配专用寄存器；不够则部分进内存，必要时 `LD`/`ST`
2. 用模板把赋值/运算译成指令
3. 按块发射，加标签与分支
4. 重排基本块以尽量消除无条件跳转
5. 目标相关窥孔优化

## 30. 综合 I（Putting It All Together I）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/557a429644ca286f12a8661b4a1f9026_Slide31.png" alt="Putting It All Together I" width="80%"/>

GCD 的原 CFG 与略优化 CFG：主要是常量传播/折叠。顶块关于变量的知识不能简单传到多前驱的 `if` 块。

## 31. 综合 II（Putting It All Together II）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5667f4503d1c5154b88e7ba16ecd7e52_Slide32.png" alt="Putting It All Together II" width="80%"/>

为 `x`、`y` 专配寄存器；按块生成；重排消除无条件分支。结果已接近人手难再明显改进的质量。

## 32. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1ab32272df7cf36304a2526469a78a24_Slide33.png" alt="Summary" width="80%"/>

编译流水线按序把源码变为高质量汇编：词法 → 语法 → 语义 → IR/CFG 优化 → 代码生成。耐心多遍优化常优于手写汇编；程序员专注功能正确性，细节交给编译器。
