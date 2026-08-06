---
title: MIT 6.004：L17 处理器虚拟化
date: 2026-08-11 10:17:00
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L17 注解幻灯片。
>
> 源网页：[17.1 Annotated Slides | Virtualizing the Processor](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c17/c17s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L17：处理器虚拟化（Virtualizing the Processor）

本讲在虚拟内存之上引入 **process** 与 **虚拟机** 抽象：用定时器中断做 **timesharing**、在 kernel/user 模式间切换保存恢复状态；并用非法指令异常做指令仿真与 **SVC**（supervisor call）系统调用。

## 1. 回顾：虚拟内存（Review: Virtual Memory）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ebe3973c0da509604a394edbd37ee0e4_Slide02.png" alt="Review Virtual Memory" width="80%"/>

上讲引入虚拟内存与 **MMU**：CPU 虚地址 → 主存物理地址，多程序可各享独立大地址空间。虚/实空间都按页划分；例：页 $2^{12}$ 字节、32 位地址 → $2^{20}$ 页，高 20 位页号、低 12 位偏移。

MMU 用页表把 VPN 映到 PPN（常多层，仅驻留活跃部分）；**TLB** 缓存近期翻译。已分配虚存内容在辅存；不在主存则 **page fault**，OS 装页。实践中每程序仅活跃页驻留主存。

## 2. MMU 地址翻译（MMU Address Translation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7d3fc5431fbc939b402cb22d4c985231_Slide03.png" alt="MMU Address Translation" width="80%"/>

先查 TLB；未命中再 walk 分层页表；不驻留则 page fault。映射 context 由两寄存器控制：**context-number**（TLB 可见哪些映射）与 **page-directory**（页目录所在物理页）。重载二者即可换 context。

多 context 需要够大的 TLB 同时缓存各进程热点映射，以及若干物理页存放页目录/页表段。例：两端各约 1024 页的两级页表用 3 页即可覆盖约 8MB 代码/栈/堆——对许多简单程序足够。

## 3. Context（Contexts）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dbef9fc552cafe1f3db2ed4c7a0470bc_Slide04.png" alt="Contexts" width="80%"/>

页表构造虚→实翻译所需的 context。多任务希望支持多 context 并快速切换，从而共享物理内存：两程序都可把虚地址 0 当入口，却落到不同物理页。换程序时做 **context switch**。接下来弄清如何共享 CPU。

## 4. 构建虚拟机（Building a Virtual Machine (VM)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6ad8a765f3284bdba65e3b7bd486d584_Slide05.png" alt="Building a Virtual Machine VM" width="80%"/>

抽象 **process**（进程）= 正在运行的程序及其资源（CPU、MMU、I/O 等）。**进程状态**包括：

- CPU 硬件状态：寄存器与 PC
- 虚地址空间内容：代码、数据、栈、堆对象（可在主存或辅存）
- MMU 状态：context-number、page-directory，以及分层页表占用的页
- I/O 相关：文件读写位置、网络缓冲、键盘/鼠标事件等

特权进程 **OS** 跑在 kernel context，记账并周期调度各进程，提供文件、网络、窗口等服务。换用户进程须保存/恢复**完整**状态（主存中已有部分、内核数据结构、CPU/MMU 硬件）。目标：让每进程以为独占一台独立 **虚拟机**，高效共享一台物理机。

## 5. 每进程一台 VM（One VM For Each Process）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/25ce2769ddc80e6ff5f423974196a881_Slide06.png" alt="One VM For Each Process" width="80%"/>

底层是物理机：CPU + 主存，外加外设（timer、辅存、USB、网络、显示器/键鼠等）。OS 在特权 kernel context 管理外设与 MMU，为每进程造出虚拟机。

用户代码直接在物理 CPU 上跑，但可被 timer 打断，使 OS 保存当前进程、切换下一进程。经 MMU，每进程有隔离的虚地址空间。OS 提供的虚拟外设屏蔽共享细节（窗口像素、键盘焦点归属哪个进程等）：进程看到的是 I/O 事件流，而非直接操纵设备。

## 6. 进程：多路复用 CPU（Processes: Multiplexing the CPU）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ad7cab832e2aecb3b5a867db6c8d474e_Slide07.png" alt="Processes Multiplexing the CPU" width="80%"/>

从进程 #0 切到 #1：用户态执行被 yield 或更常见的 **timer interrupt** 打断 → 进内核（PC+4 存入 XP）。OS 把 #0 状态写入内核表，再装入先前保存的 #1 状态，`JMP` 回到用户态——#1 从上次被打断处继续。轮转各进程。

对进程而言，**虚拟时间**只是指令序列；若不看实时钟，感觉不到偶尔被挂起。从外部看，真实时间在进程间与 OS 切换间穿梭。CPU 时间多路复用称 **timesharing**。

## 7. 关键技术：定时器中断（Key Technology: Timer Interrupts）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d6670fe413417975b9fda1da9bb29dd2_Slide08.png" alt="Key Technology Timer Interrupts" width="80%"/>

外设断言 Beta 的 **IRQ**。若在 user mode（PC 中 supervisor 位为 0），识别中断的那拍：强制部分控制信号——`PCSEL=4` 选内核入口（timer 为 `0x80000008`，同时 PC[31]=1 进入 kernel）；`WASEL/WDSEL/WERF` 把 PC+4 写入 XP（R30）；`MWR=0` 以正确中止可能正在进行的 ST。下一拍从中断处理程序第一条指令开始。

## 8. Beta 中断处理（Beta Interrupt Handling）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7a3b021599c4998f58012f9ee89a6ce5_Slide09.png" alt="Beta Interrupt Handling" width="80%"/>

硬件极简：存 PC+4 到 XP，并把 PC 设为依中断类型而定的入口。其余由软件完成：把 R0–R30 存入内核结构 **UserMState**，再调 C 处理函数；返回后从 UserMState 恢复，XP 减 4 指向被打断指令，`JMP(XP)` 回用户态。

简单 Beta 把各类中断入口放在连续字（reset→0，非法指令→4，timer→8…），第一条常是跳到真正处理代码；PC[31]=1。也可在已知地址放向量表，由硬件取入口——功能等价。因保存/恢复完整状态，中断对用户程序**透明**。

## 9. 例：定时器中断处理（Example: Timer Interrupt Handler）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ae8664cf9dfb250d4b23116e9ece7ed2_Slide10.png" alt="Example Timer Interrupt Handler" width="80%"/>

先用 timer 更新 OS 中的 **TOD**（time of day），设每 1/60 秒中断一次。用户程序无需特殊处理；周期进入内核时钟处理再恢复，宛如未发生。若需 TOD，向 OS 发服务请求。汇编桩负责保存/恢复状态，中间调 C 过程。

## 10. 中断处理代码（Interrupt Handler Coding）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bb281cd561578935de610a93a0d44148_Slide11.png" alt="Interrupt Handler Coding" width="80%"/>

C 侧声明 TOD、`UserMState` 与递增 TOD 的过程。地址 8 的 `BR` 转到 `CLOCK_H`：保存寄存器（R31 恒 0 可不存）、建内核栈、调 C；返回后恢复寄存器，XP-=4，`JMP(XP)`。

与分时的联系：在 timer handler 里每隔若干次（如 `QUANTUM`）调用 **Scheduler()**——分时魔法发生处。

## 11. 简单分时调度（Simple Timesharing Scheduler）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f1bf8de120b832b3c8669cf9d70b995b_Slide12.png" alt="Simple Timesharing Scheduler" width="80%"/>

`UserMState` 暂存中断期间的用户寄存器；**PCB**（process control block）数组为每进程长期保存完整状态：寄存器副本、MMU 状态、I/O（如虚拟控制台编号）等。`CUR` 指向当前进程。

`Scheduler()`：把暂存状态写入当前 PCB → `CUR` 轮转到下一进程（到尾回 0）→ 从新 PCB 装入暂存区并配置 MMU → 返回后 clock handler 把更新后的状态装回 CPU 并恢复执行。于是换到新进程。

## 12. OS 组织：进程（OS Organization: Processes）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e55cb228555ac05a7c3a0b41934740fc_Slide13.png" alt="OS Organization Processes" width="80%"/>

再走一遍：timer 打断用户程序 → 进 clock handler → 寄存器进 `UserMState` →（若调用）`Scheduler` 写入当前 PCB、装入下一进程暂存 → handler 装回 CPU → 从新进程继续。

## 13. 一次一个中断（One Interrupt at a Time）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/51a97449df75ff4c76344c6f4c239e51_Slide14.png" alt="One Interrupt at a Time" width="80%"/>

内核 supervisor 位为 1 时**关中断**，避免嵌套中断覆盖 `UserMState`。因此 OS 代码须极度小心：死循环无法被打断，机器像“冻住”，只能断电重启。用户态允许中断，失控程序仍可被键盘等打断；OS 常有热键挂起当前进程并可选保存现场供调试。

## 14. 异常硬件（Exception Hardware）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b878b7906da283f09b440eed109cbf85_Slide15.png" alt="Exception Hardware" width="80%"/>

OS 还处理“非法”操作码（硬件未直接实现的操作，也称 **UUO**）。行为类似中断，但是 CPU 自身触发：挂起当前指令，把 PC+4 写入 XP，PC←`0x80000004`（含 supervisor 位），进入内核处理。可用软件**仿真**扩展指令集。

## 15. 异常处理（Exception Handling）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3d22bc6b9aaa3fa0264ceb19c5a3df47_Slide16.png" alt="Exception Handling" width="80%"/>

类似实验用 TinyOS：地址 0 起是各类中断/异常的分支；非法指令走位置 4 的 `BR(I_IllOp)`。其后分配 OS 栈、`UserMState`、进程表等数据结构。

## 16. 实用宏（Useful Macros）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9522bb18e4d72ee1b12b8f1c4a53d3d2_Slide17.png" alt="Useful Macros" width="80%"/>

汇编里用宏展开重复序列。例：从 32 位数提取比特域 $[N..M]$（bit 31 为 MSB）。另有宏把 CPU 寄存器批量保存到 / 从 `UserMState` 恢复。

## 17. IllOp 处理（Illop Handler）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/92d986674ebe1d14073fc3fd6632d153_Slide18.png" alt="Illop Handler" width="80%"/>

标准开头：保存用户寄存器、初始化 OS 栈。取出非法指令——保存的 PC+4 是**用户虚地址**，须经 MMU 例程换成物理地址再读。用 opcode 索引 **dispatch table**（64 项）跳到对应处理：多数进 `UUOError`；opcode 1 作 **supervisor call**；opcode 2 仿真 `SWAPREG`。表驱动分派通常比长串比较更省时省空间。

## 18. 访问用户地址（Accessing User Locations）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ae7db9b0d6370c1dd569de0b6e662485_Slide19.png" alt="Accessing User Locations" width="80%"/>

基于上讲 **VtoP**：把 VPN 与偏移按约定压栈，返回物理地址于 R0，再以物理地址读主存中的用户位置。

## 19. 真正非法操作码（Handler for Actual Illops）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/87c9f660315ff45bd58eaf4257236658_Slide20.png" alt="Handler for Actual Illops" width="80%"/>

对真正非法的 opcode：打印错误信息并崩溃（如 Windows “蓝屏”）。更好做法：把进程状态写入调试文件（历史称 **core dump**），终止该进程并提示用户，稍后再用调试器查 dump——而非拖垮整机。

## 20. 仿真指令：swapreg（Emulated Instruction: swapreg(Ra,Rc)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/063af60e32a9c5a351460b64c8f8e5c4_Slide21.png" alt="Emulated Instruction swapreg" width="80%"/>

`SWAPREG` 交换两寄存器。先让汇编器把 `swapreg(ra,rc)` 编成类似 ADDC 的二进制（literal=0，opcode=2）。仿真：从指令取出 RA/RC，换成 `UserMState` 数组字节偏移，交换暂存中的用户寄存器值；返回时装回 CPU，程序仿佛硬件执行了该指令。

## 21. 与 OS 通信（Communicating with the OS）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6d120de61ef824b804968c79bb355cf6_Slide22.png" alt="Communicating with the OS" width="80%"/>

用户与 OS 不同 MMU context，不能直接碰 OS 代码/数据（也不应绕过安全策略）。需要在明确入口调用 OS，经寄存器或用户虚存传参——即 **supervisor call / SVC**，构成受控 API（如 POSIX）。

约定：opcode=1 的非法指令作 SVC，低位字段索引具体服务。

## 22. OS 组织：SVC（OS Organization: Supervisor Calls）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c81038573a315480603fe64210cba75b_Slide23.png" alt="OS Organization Supervisor Calls" width="80%"/>

用户程序执行不同索引的 SVC → 硬件当非法指令进 IllOp → 保存状态 → 按 opcode 分派；SVC 子处理可读用户寄存器或用户虚地址，返回值可写回暂存（如覆盖保存的 R0）→ 恢复寄存器，从 SVC **下一条**继续。

## 23. SVC 处理（Handler for SVCs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/caac10bd3f504fcebbac1ce574035f3a_Slide24.png" alt="Handler for SVCs" width="80%"/>

opcode=1 的子处理再用指令低位索引第二张 dispatch 表（TinyOS 仅少数简单服务）。真实 OS 会有文件、网络、虚存、创建进程等大量 SVC。

## 24. 返回用户态（Returning to User-mode）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fe48584cc38c4c9a507537f8f5451ade_Slide25.png" alt="Returning to User-mode" width="80%"/>

完成则恢复寄存器并对 XP 指向的下一条 `JMP`。若暂时无法完成（如 `ReadCh` 尚无字符）→ 转 `I_Wait`：安排下次再执行该 SVC，并 `Scheduler()` 让其他进程跑。

同套代码也实现：`Yield()` 主动放弃本时间片；`Halt()` 名不副实——每次被调度到都重做 Halt SVC 再调度别人，后续指令永不执行，表现为停机。

## 25. 添加新 SVC（Adding New SVCs）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8c447300eb481650620c299f41126888_Slide26.png" alt="Adding New SVCs" width="80%"/>

步骤：为用户程序定义新 SVC 宏（如 get/set TOD）；微调 SVC 分派范围；在 dispatch 表末尾加项。

## 26. 新 SVC 处理程序（New SVC Handlers）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/93391eba1ea88886265bce2a3f5e2ff4_Slide27.png" alt="New SVC Handlers" width="80%"/>

处理程序经 `UserMState` 读写用户 R0 等，几条指令即可完成。SVC 提供受控的 OS 服务入口；且在 supervisor 模式关中断，处理程序**不可被打断**——若需 `LD/ADDC/ST` 原子递增主存变量，可封装为 SVC。本讲为简单分时 OS 打下基础；下讲继续看 OS 如何与外部 I/O 设备交互。
