---
title: MIT 6.004：L18 设备与中断
date: 2026-08-11 10:18:00
categories: ic
tags:
  - ic
  - digital-circuit
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L18 注解幻灯片。
>
> 源网页：[18.1 Annotated Slides | Devices and Interrupts](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c18/c18s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L18：设备与中断（Devices and Interrupts）

本讲讲 OS 如何用 **interrupt** 与内核缓冲对接 I/O，以及 **blocking SVC**（ReadKey）在中断禁用下的实现演进（忙等 → 重试 SVC → Scheduler → sleep/wakeup）；后半转入 **hard real-time**：latency、weak/strong priority、周期性负载与截止期可满足性。

## 1. OS 组织：I/O 设备（OS Organization: I/O Devices）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6f340663bd0482a4d24e623d39a66b0c_Slide02.png" alt="OS Organization: I/O Devices" width="70%"/>

OS 与外设交互分两层：

1. **设备侧**：interrupt handler + 内核缓冲，把数据搬进/搬出设备
2. **用户侧**：supervisor call（SVC）按用户进程请求访问这些缓冲

难点：SVC 发出时请求未必立刻能完成（缓冲空/满），需要阻塞与唤醒机制。

## 2. 异步 I/O 处理（Asynchronous I/O Handling）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/387d4bbbd189f6a7b05c2d0a9c254f07_Slide03.png" alt="Asynchronous I/O Handling" width="70%"/>

流程：按键 → 键盘触发 interrupt → 暂停当前进程 → handler 读字符写入**拥有键盘焦点**进程的内核缓冲 → 恢复被中断进程。人打字远慢于指令执行，及时服务即可跟上。

缓冲满时：覆盖旧字符无意义，通常丢弃新字符并蜂鸣提示。稍后用户程序调用 `ReadKey()` SVC，OS 从缓冲取字符放入用户 R0。

- **blocking I/O**：返回时 R0 必有字符；尚无则阻塞
- **non-blocking I/O**：立即返回状态标志 + 结果，程序自行决定是否稍后重试

## 3. 基于中断的异步 I/O（Interrupt-based Asynch I/O）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/53d6b3da4683a38cfec92a08402bb9bf_Slide04.png" alt="Interrupt-based Asynch I/O" width="70%"/>

用户程序**不轮询**键盘，而是 **event-driven**：设备需要服务时用 interrupt 通知 OS。职责分离优雅——有活才占 CPU，对用户程序透明。

设备访问两种常见方式：

- **专用 I/O 指令**（如实验 Beta 的 `RDCHAR()`、`CLICK()`）
- **memory-mapped I/O**：内核地址空间一段映射到设备寄存器，用普通 LD/ST 访问

示意代码用 MMIO：结构体含 status 与 data；handler 读键码写入环形缓冲。实际还要处理 press/release、Shift/Ctrl、CTRL-ALT-DEL 等特殊组合，以及 raw vs cooked 输入模式。

## 4. ReadKey SVC：尝试 #1（ReadKey SVC: Attempt #1）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b4e80e824b42f396ae873213ac39eb99_Slide05.png" alt="ReadKey SVC: Attempt #1" width="70%"/>

`ReadKey()` 的 opcode 非法 → 异常进入 OS → 识别为 SVC 再分派子 handler。缓冲非空：从进程表找到对应键盘缓冲，字符写入保存的用户 R0，退出后恢复寄存器。

缓冲空时若在 handler 内 `while` 空转等待：SVC 在 **supervisor 模式**（PC[31]=1）运行，**中断被禁用** → 键盘 interrupt 永远进不来 → 死循环，系统假死。失败。

## 5. ReadKey SVC：尝试 #2（ReadKey SVC: Attempt #2）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f8097de1f3ee1fd7ac973f07004e35a2_Slide06.png" alt="ReadKey SVC: Attempt #2" width="70%"/>

修复：返回前把保存的 XP 减 4。异常时 XP 存的是非法指令的 PC+4；正常 `JMP(XP)` 会执行 SVC **之后**的指令。XP−4 使恢复后**重新执行同一条 SVC**。

关键差别：重试时有一拍在 **user-mode**（PC[31]=0）执行，此时若有挂起键盘 interrupt，可抢占并填满缓冲；再进 SVC 就能取到字符。能工作，但空缓冲时忙等重试，浪费 CPU，直到 timer 切到别的进程。

## 6. ReadKey SVC：尝试 #3（ReadKey SVC: Attempt #3）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e726ce3a007193dd3d2b4544c117802e_Slide07.png" alt="ReadKey SVC: Attempt #3" width="70%"/>

空缓冲时：安排重试 SVC 后立刻调用 `Scheduler()`，主动让出时间片。轮转调度回来再试。代价是打字后重启略有延迟，但时间片通常短于击键间隔，不明显。

对照 **timesharing 质疑**：10 个各需 1s 的作业，无分时依次完成；有分时则几乎都在 ~10s 后才完——最坏完成时间不更短。但若多数进程在等 I/O，分时把空闲周期送给能干活的进程，整体利用率更好。真实系统里大量进程处于 I/O wait。

## 7. 更精细的调度（Sophisticated Scheduling）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1eca87babc2c8ec8059184eacff362fa_Slide08.png" alt="Sophisticated Scheduling" width="70%"/>

进程状态加 **status**：ACTIVE（0）或 WAITING（非零，不同值表示等不同事件）。`Scheduler()` 只跑 ACTIVE。UNIX 风格原语：`sleep(event)` / `wakeup(event)`，参数即 status 标识。

## 8. ReadKey SVC：尝试 #4（ReadKey SVC: Attempt #4）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/12ac12fc71aee107e0c06f8ff736ec67_Slide09.png" alt="ReadKey SVC: Attempt #4" width="70%"/>

缓冲空 → `sleep(kbdnum)` 设 WAITING 并调度；键盘 handler 写入缓冲后 `wakeup(kbdnum)`，把所有等该事件的进程标为 ACTIVE。睡着的进程在事件发生前**完全不占 CPU**——优雅且高效。

## 9. 例题：Handler 与 OS 配对（Example: Match Handler to OS）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3716d791a46f800e836813d2102d8694_Slide10.png" alt="Match Handler to OS" width="70%"/>

三种 ReadKey 变体 R1/R2/R3，三种系统 Model A/B/C：

- **R1**（似尝试 #2，但总读键盘 0）：只适合单进程 **Model C**（分时下会串共享输入）
- **R2**（似尝试 #1 的 while 空转）：只适合 **Model B**（SVC 内仍允许设备中断）
- **R3**（尝试 #3）：配对标准 **Model A**（内核不可中断）

## 10. 哪套 Handler 与 OS？#1（Which Handler and OS? #1）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5fdb0e8375bb945cee37b58830c98806_Slide11.png" alt="Which Handler and OS #1" width="70%"/>

用户报：“编译错误，Scheduler 与 ProcTbl 未定义。” → 非分时系统无这两符号；R2 也不调用 Scheduler → **R3 跑在 Model C**。

## 11. 哪套 Handler 与 OS？#2（Which Handler and OS? #2）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/870e378c81686dc62176636fee65a36c_Slide12.png" alt="Which Handler and OS #2" width="70%"/>

“现在总从键盘 0 读所有人输入，而且更浪费 CPU。” → 只有 R1 固定键盘 0；相对旧 handler 明显更费说明以前不是忙等的 R2 → **R1 在 Model A**。

## 12. 哪套 Handler 与 OS？#3（Which Handler and OS? #3）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/faa9b5bd806b1ced68b275260871015e_Slide13.png" alt="Which Handler and OS #3" width="70%"/>

“新系统工作正常，还更省 CPU！” → 排除 R1 在分时（共享键盘可察觉）、R2/R3 在无进程表的 C、R2 在不可中断内核 A → **Model B 用户现跑 R3**。

## 13. 对“实时”的需求（The Need for “Real Time”）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/26fabb30632dc32ef78d66826bf4e9f2_Slide14.png" alt="The Need for Real Time" width="70%"/>

分时给每进程独立虚拟机的错觉，利用率好，但**无法保证完成时间**——取决于其他进程占用。OS 把 interrupt 事件暂存与用户态处理（经 SVC）分离，更难保证在 **deadline** 前处理完。

汽车 ESC（电子稳定控制）等控制系统有硬截止期：测力/转向/轮速 → 判是否失控 → 单轮制动纠正航向。错过截止期可能危及安全。

## 14. 中断延迟（Interrupt Latency）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/be096a683234f957ad7dc43069022ece_Slide15.png" alt="Interrupt Latency" width="70%"/>

**Interrupt latency** $L$：从请求运行某代码到代码真正开始执行的时间。服务时间 $S$、截止期 $D$ 时，最大允许延迟满足 $L_{\max}+S=D$。必须始终 $L < L_{\max}$ 的约束称 **hard real-time constraints**。

## 15. 延迟来源（Sources of Interrupt Latency）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/274de239122301c4043bbdbe0b3ee617_Slide16.png" alt="Sources of Interrupt Latency" width="70%"/>

贡献因素：保存进程状态、切内核、分派 handler；不可中断的长时段（复杂多周期指令如 block move——ISA 应可中断重启）；以及**已在内核处理另一 interrupt**（中断禁用）时的等待。

目标：界住并最小化 $L$——优化取中断路径、避免数据相关长指令、缩短内核态时间；必要时允许内核内仍可被更高优先级中断。

## 16. 多设备调度（Scheduling of Multiple Devices）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fa87034007c412f4f098ae7bcede8d6c_Slide17.png" alt="Scheduling of Multiple Devices" width="70%"/>

三设备服务时间：键盘 800 µs、磁盘 500 µs、打印机 400 µs。请求稀少、任意到达。**FCFS** 下最坏延迟 = 另两设备服务之和：键盘 900、磁盘 1200、打印机 1300 µs。长 handler 拖累他人——有无更好调度？

## 17. 弱（非抢占）优先级（Weak (Non-preemptive) Priorities）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7b7c68dda40549f9ee57ad07392bfc98_Slide18.png" alt="Weak Priorities" width="70%"/>

**Weak / nonpreemptive priority**：当前任务跑完才按优先级选下一个；新到的更高优先级也不能打断。最坏延迟 ≈ 任意其他设备最长服务时间 + 所有更高优先级服务时间。

例：优先级 磁盘 > 打印机 > 键盘 → 键盘仍 900；磁盘只需等当前最长者（键盘）→ **800 µs**；打印机最坏 800+500=**1300 µs**。

## 18. 设定优先级（Setting Priorities）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9c4789a9f8a37aa9de5a4d8a33081631_Slide19.png" alt="Setting Priorities" width="70%"/>

硬实时下：**Earliest Deadline**——按截止期排序，越早截止优先级越高。若存在能满足所有截止期的优先级赋值，EDF 也能满足。机场安检先办最早航班的比喻。过载时最小化“误机人数”则更复杂，超出本讲范围。

默认：未另行规定时，$D$ 取为到**同一设备下一次请求**的时间，避免系统越落越远。

## 19. 需要抢占（The Need for Preemption）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9b2b44ae04af2fcbc6f05b6b0217d37d_Slide20.png" alt="The Need for Preemption" width="70%"/>

弱优先级下最坏延迟总含“当前最长其他任务”。若磁盘截止期 800 µs、$S=500$ → 允许 $L_{\max}=300$ µs，但弱优先级只能保证 800 µs → **不够**。

引入 **strong / preemptive priority**：高优先级可打断低优先级 handler。同优先级序下：磁盘 $L=0$，打印机 500，键盘仍 900——磁盘截止期可满足。

## 20. 强优先级实现（Strong Priority Implementation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/120b86a7068805a8ecaceac57891d61d_Slide21.png" alt="Strong Priority Implementation" width="70%"/>

Beta 改造：PC[31] 单比特 supervisor 改为 **PC[31:29] 的 3-bit PRI**（8 级）。设备请求带自己的优先级 $P_{\mathrm{DEV}}$；优先级编码器选出最高请求，仅当 $P_{\mathrm{DEV}} > \mathrm{PRI}$ 才接受中断。接受后旧 PC+PRI 存入 XP，新 PRI 设为 $P_{\mathrm{DEV}}$。高优先级最坏延迟**不再受**低优先级服务时间影响。

## 21. 重复中断（Recurring Interrupts）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/605e7ac85d42410cabd443760c8b067f_Slide22.png" alt="Recurring Interrupts" width="70%"/>

加上最大请求频率：打印机每 1 ms、磁盘 2 ms、键盘 10 ms。强优先级下磁盘立即服务、打印机可抢占键盘。键盘**开始**延迟仍可 ≤900 µs，但不断被抢占，**完成**可能晚到请求后 3 ms——说明实时约束应用 **deadline** 表达，而非仅看 latency。周期需求过紧时 CPU 周期根本不够。

## 22. 中断负载（Interrupt Load）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fd13af21837881c7ab3965baeb1c7927_Slide23.png" alt="Interrupt Load" width="70%"/>

周期负载：键盘 $800\,\mu\mathrm{s}/10\,\mathrm{ms}=8\%$，磁盘 25%，打印机 40%，合计 **73%**，剩 27% 给用户态。总负载 >100% 必失败。

截止期窗口内还要为更高优先级留预算：磁盘需 $500/800\approx 67.5\%$；打印机有效截止 1000 µs 内需 500+400=900 µs。键盘若 $D=2000$ µs 需 500+2×400+800=2100 > 2000 → 不可行；$D=3000$ 时 2×500+3×400+800=3000 → 刚好。

## 23. Mr. Blue 访问 ISS（Mr. Blue Visits the ISS）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bf4a0a0a85f1e8bbaf973df095ba7e13_Slide24.png" alt="Mr. Blue Visits the ISS" width="70%"/>

国际空间站三任务 SSG / G / CP；先分析 **弱优先级**：

1. CP 最大服务时间：G 的 $L_{\max}=10$ ms → 任何其他 handler ≤10 ms
2. EDF 序：G > SSG > CP
3. 负载：SSG 5/30≈16.7%，G 25%，CP 10% → 合计 ~51.7%，空闲 ~48.3%
4. 最坏完成：SSG 等 CP+G 再加自身 → 25 ms；G 等 CP+自身 → 20 ms；CP 等 SSG+G+自身 → 25 ms

## 24. Mr. Blue 访问 ISS（续）（Mr. Blue Visits the ISS (cont’d.)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2543f7677cf0becbcc4f956406e7275d_Slide25.png" alt="Mr. Blue Visits the ISS cont" width="70%"/>

改 **强优先级**（同序 G > SSG > CP）：

1. CP 可被抢占 → 不再受高优先级 $L_{\max}$ 限制；100 ms 窗口内最多 4 次 SSG + 3 次 G = 50 ms，故 CP 服务可达 **50 ms**
2. CP 占 50% + 其余 → 总约 91.7%，空闲 ~8.3%
3. 最坏完成：G = 自身服务时间；SSG ≤ 一次 G + 自身 = 15 ms；CP 按设计刚好压在 100 ms 截止期

## 25. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8f63a010fb3153ee485dd60154fdd739_Slide26.png" alt="Summary" width="70%"/>

- 用户与设备交互拆成：设备侧 interrupt + 内核缓冲；应用侧 SVC
- 阻塞 I/O：经 XP−4 重试、Scheduler、最终 **sleep/wakeup**，避免空转
- Hard real-time：latency、service time、deadline；**weak** vs **strong** priority
- 实践中常多级强优先级，同级内再用弱优先级仲裁——足以应对多数 I/O 实时约束
