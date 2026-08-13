---
title: MIT 6.004：L06 有限状态机
date: 2026-08-11 10:06:00
categories: ic
tags:
  - ic
  - digital-circuit
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L06 注解幻灯片。
>
> 源网页：[6.1 Annotated Slides | Finite State Machines](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c6/c6s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L06：有限状态机（Finite State Machines）

本讲在时序逻辑之上引入 **FSM** 抽象：状态转移图、Moore/Mealy、ROM/门级实现、等价状态化简，以及异步输入与亚稳态、同步器隔离策略。

## 1. 我们的新机器（Our New Machine）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/68c02542c606a9dfbeb43171bcbb9d61_Slide02.png" alt="Our New Machine" width="80%"/>

上一章得到时序逻辑：组合逻辑云 + 状态寄存器。

- **组合逻辑**：无环图，服从静态纪律——合法且稳定的数字输入 → 在最后一次输入跳变后的规定时间内得到合法且稳定的数字输出；功能规格给出每种输入组合的输出。图中有 $k+m$ 个输入、$k+n$ 个输出，真值表有 $2^{k+m}$ 行、$k+n$ 列输出。
- **状态寄存器**：记住当前状态，用 $k$ 比特编码，最多 $2^k$ 个互异状态。状态以合适方式捕获输入序列中**相关历史**；过去输入若影响后续行为，正是通过这些状态比特。典型地，周期时钟上升沿触发 LOAD，把组合逻辑算出的新状态写入寄存器。

设计任务：决定期望输入序列应对应怎样的输出序列（一次输入可能引出一长串输出，或输出暂不变、靠内部状态逐步记住信息）→ 写出计算输出与下一状态的功能规格 → 画出实际电路。

## 2. 一个简单的时序电路（A Simple Sequential Circuit）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/88e72bf365ab07be625d0296d1c76110_Slide03.png" alt="A Simple Sequential Circuit" width="80%"/>

例：密码锁。1 比特输入按位串入口令；输出 UNLOCK 仅当最近四位输入为 `0-1-1-0` 时为 1。

是否必须记住最近四位（4 个状态比特）？不必。只需知道“最近输入是否构成正确口令的某个前缀”——输入不对时，不必记录“错成什么样”，只需知道“不对”。由此引入描述时序行为的抽象。

## 3. 今日抽象：有限状态机（Abstraction du jour: Finite State Machines）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/cf9238cb993345df80ba1e4ad7babb25_Slide04.png" alt="Finite State Machines" width="80%"/>

**FSM** 描述时序逻辑的输入/输出行为，与具体实现无关。要素：

- **周期 CLOCK**：上升沿触发当前状态 → 下一状态
- **有限个状态**，上电有一个初始/起始状态。状态数本身是设计题：状态比特数与计算下一状态/输出的组合逻辑复杂度常有折衷
- **输入**：传送外部信息。100 比特信息可一次并行送入，也可单线串行 100 拍——时序逻辑远快于被控物理过程时，常选**比特串行**以省互连，代价是传输时间
- **输出**：同理可串行/并行编码
- **转移规则**：由当前状态 $S$ 与输入 $I$ 定下一状态 $S'$，须对所有 $S,I$ 组合穷尽
- **输出规则**：常可仅是 $S$ 的函数（更简单）；一般也可是 $S$ 与当前输入的函数

## 4. 状态转移图（State Transition Diagram）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c5025083ffedf14759dbfd3c0cdc6162_Slide05.png" alt="State Transition Diagram" width="80%"/>

用状态转移图描述密码锁 FSM。初始状态 SX：尚未收到任何口令位；圆圈表示状态，符号名提醒“记住了什么历史”。本例解锁输出 $U$ 仅依赖当前状态，写在圆内；SX 中 $U=0$。初始态用粗边框标出。

收到 0 → 转到 S0（已见口令首位）。箭头标转移条件；时钟上升沿触发转移。继续补全正确口令路径，最右态 S0110 表示已检出 `0-1-1-0`，$U=1$。从 SX 输入 `0-1-1-0` 会停在 S0110。

输入不是期望下一位时：例如 SX 收到 1，仍无正确前缀，留在 SX。错误输入**不一定**回到 SX——若在 S0110 再收 1，最近四位变成 `1-1-0-1`，但末两位 `0-1` 可能是合法口令前缀，故转到 S01。

输出仅是当前状态的函数 → **Moore 机**（输出写在状态圆内）。输出还依赖当前输入 → **Mealy 机**：转移箭头上用 `/` 分隔输入与输出，如 `0/1`。

## 5. 合法的状态图（Valid State Diagrams）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a2facbc7f2e330d15206a52d1768bfb5_Slide06.png" alt="Valid State Diagrams" width="80%"/>

良构规则：

- **互斥**（mutually exclusive）：同一状态下不能有两条转移带相同输入标签——否则下一状态有歧义，行为不一致
- **穷尽**（collectively exhaustive）：每个可能输入值都应有转移；若某输入下应停留，画指向自身的自环

于是对每一对（当前状态，输入）恰选一条转移。

## 6. 状态转移图即真值表（State Transition Diagram as a Truth Table）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f5faf7a3e0e141813e04bb647c77cd0e_Slide07.png" alt="State Transition Diagram as a Truth Table" width="80%"/>

转移图可写成真值表：行 = 当前状态 + 输入的所有组合；输出列 = 下一状态与输出。用二进制替换符号状态名后，即第 4 章那种真值表。$K$ 个状态需 $\lceil\log_2 K\rceil$ 个状态比特（本例 5 态 → 3 比特）。

编码可任意（000、001…），但**状态编码**会强烈影响实现逻辑复杂度——找最简逻辑的编码本身很有趣。有了真值表，可用第 4 章方法做组合逻辑，或干脆用 ROM。

## 7. 落到硬件（Now Put It in Hardware）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4ecfb54f0d10a8e59b284a051b168b81_Slide08.png" alt="Now Put It in Hardware" width="80%"/>

ROM 由当前状态与输入计算下一状态与输出。5 态用 3 比特编码 → 3 比特状态寄存器（边沿触发矩形为多比特寄存器的示意；线上斜杠 + 数字表示位宽）。

ROM 共 4 根地址输入（3 状态 + 1 输入）→ $2^4=16$ 个单元，对应真值表 16 行；每单元 4 比特（3 下一状态 + 1 输出）→ **16×4 ROM**。状态寄存器仍须服从动态纪律（第 5 章末时序分析）；暂假定输入跳变已与时钟上升沿正确同步。

## 8. 离散状态、离散时间（Discrete State, Discrete Time）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f998b521ccfef242b7272405810c45f6_Slide09.png" alt="Discrete State, Discrete Time" width="80%"/>

回顾设计选择：输出仅依赖状态 → Moore；依赖状态+输入 → Mealy。$S$ 个状态比特可编码 $2^S$ 态。**每多 1 个状态比特，ROM 容量翻倍**——用 ROM 实现时极想减少状态比特。

波形：时钟上升沿 → 状态寄存器输出更新 → ROM 算出下一状态（本周期某时刻稳定）→ 下一上升沿装入。周而复始，跟随转移图。

## 9. 杂务问题（Housekeeping Issues…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7c5e289f2e4746879908ce5b6d79983c_Slide10.png" alt="Housekeeping Issues" width="80%"/>

- **复位**：上电需把状态寄存器置为初始态编码；常用 RESET=1 强制初态，再置 0 开始运行
- **未用编码**：3 比特最多 8 态，本例只用 5 个——可改用门级逻辑；若寄存器误入未用编码，可在 ROM 中令未用态一律指向初态，避免未知行为
- **编码优化**：CAD 可帮找使组合逻辑最简的状态编码
- **另一实现**：移位寄存器记住最近四位，再比较是否匹配口令——无需复杂的下一状态逻辑
- **异步输入**：如何保证输入跳变不破坏状态寄存器的动态纪律——本章末节讨论

## 10. FSM 的状态（FSM States）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/82a022e1f682dee47837bf697d8240c2_Slide11.png" alt="FSM States" width="80%"/>

- 用 $K$ 个状态比特的 FSM，转移图中状态数至多 $2^K$
- 两台 FSM 串联（第一台输出 = 第二台输入）：整体仍是 FSM；不知内部细节时，状态数上界为 $M\times N$（第一台可在任一 $M$ 态，同时第二台在任一 $N$ 态）。上界与输入位宽 $X,Y$ 无关——更宽输入只让转移标签更长，不直接告诉内部状态数

## 11. 我的转移图是什么？（What’s My Transition Diagram?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8b48eff61d9a84c762164b8d480a7dd3_Slide12.png" alt="What's My Transition Diagram" width="80%"/>

黑盒 FSM：按钮 0/1，灯输出。实验一小时：按 0 灯灭，按 1 灯亮。能否确定转移图？

两个图都能匹配“短时间”观察；第二个要连按 1 四次才暴露差异。**若状态数无上界，永远无法确认已穷尽全部行为。** 若知上界为 $K$，并每次从初态复位：可达态都能在少于 $K$ 步到达，枚举所有长度 ≤$K$ 的输入序列即可保证访问全部状态。

## 12. FSM 等价（FSM Equivalence）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4b74e046d7d92fd6b1e17a59dbb1b0a2_Slide13.png" alt="FSM Equivalence" width="80%"/>

不同状态数的 FSM 可能等价：对外不可区分即可互换。定义：当且仅当**任意输入序列**在两机上产生**相同输出序列**时，两 FSM 等价。工程上希望找最简（最便宜）的等价机——下一例说明如何化简。

## 13. 造一只机器蚁（Let’s Build a RoboAnt）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/58f824dca5002a86498e553135c0c9fb_Slide14.png" alt="Let's Build a RoboAnt" width="80%"/>

机器蚁以 FSM 为脑：触角 L、R（触碰为 1）；输出 F 前进、TL/TR 左转/右转。同时请求转向与前进时先转向。触角贴墙时可转，但无法前进。任务：在无孤岛墙的迷宫中用**右手定则**逃出。

## 14. 迷失太空（Lost in Space）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/03a01995c5d440e95efed6fbf6e7a4fb_Slide15.png" alt="Lost in Space" width="80%"/>

初态 LOST：断言 F，一直前进直到至少一根触角碰到墙（$L$ 或 $R$ 为 1）。

## 15. 撞上了！（Bonk!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d7b3405b48b09dd27733e759ffc5f9a4_Slide16.png" alt="Bonk" width="80%"/>

碰到墙后的三种姿态：为实现右手定则，逆时针左转直到两触角都离开墙 → 增加 rotate-counterclockwise 态，断言 TL，直到 $L=R=0$。

## 16. 稍向右转…（A Little to the Right…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7519e2a0b7df45b60ec57efd12d772b3_Slide17.png" alt="A Little to the Right" width="80%"/>

墙已在右侧，开始用右触角沿墙走：WALL1 断言 TR+F，然后看右触角决定下一步（期望立刻再碰到墙）。

## 17. 再稍向左转…（Then a Little to the Left…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6344748c19c9c4768d902ae21a7134cf_Slide18.png" alt="Then a Little to the Left" width="80%"/>

右触角如期碰到 → WALL2：TL+F，再检查触角。右触角仍碰 → 继续转；左触角碰 → 内角，回到 rotate-counterclockwise 把新墙放到右侧；两触角都空 → 回到 WALL1（与墙平行）。期望沿墙时 WALL1/WALL2 交替；遇内角则旋转后继续。

## 18. 处理外角（Dealing With Outside Corners）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6ff1c0db62000b3af3fb1483b3626e57_Slide19.png" alt="Dealing With Outside Corners" width="80%"/>

WALL1 中前进并右转后，外角处右触角可能碰不到墙。策略：CORNER 态继续右转并前进，直到右触角再次碰到拐角后的墙，再转入 WALL2。

## 19. 等价状态化简（Equivalent State Reduction）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b0e06d16783d8e49a9b1ceccf3f9ea54_Slide20.png" alt="Equivalent State Reduction" width="80%"/>

找可合并、对外不可区分的状态对。两态等价当且仅当：

1. **输出相同**（输出对外可见）
2. 对每种输入组合，二者转到**等价**的下一状态

策略：反复找等价对并合并，直到不能再合并。

## 20. 进化一步（An Evolutionary Step）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/16fdf264e95bd81906069a2aa3373373_Slide21.png" alt="An Evolutionary Step" width="80%"/>

蚁 FSM 中仅 WALL1 与 CORNER 输出相同（皆 TR+F）。二者转移只依赖 $R$：$R=0$ 都到 CORNER，$R=1$ 都到 WALL2 → 等价，可合并为 WALL1。

5 态 → 4 态；实现从 3 状态比特降到 2 比特——**少 1 比特，ROM 减半**。与布尔化简类似，合并等价态可省时序硬件。

## 21. 建转移表（Building the Transition Table）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6e40d4cd92520f1af527e8c0964d17c4_Slide22.png" alt="Building the Transition Table" width="80%"/>

不用 ROM、改用门实现时：把转移图逐条写入真值表。LOST（编码 00）：$F=1$；$L=R=0$ 则下一态仍 LOST；任一触角碰 → 转到 rotate-counterclockwise（01），对应 $L,R$ 的三种组合各占一行。再类似地填完其余状态的转移。

## 22. 实现细节（Implementation Details）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/65646bfa68276c029273f17c3296906f_Slide23.png" alt="Implementation Details" width="80%"/>

完整表可用 don't-care 压缩行数。再对组合逻辑各输出（2 个下一状态比特 + 3 个运动输出）写布尔方程。下一状态比特用第 4 章 K-map 找素蕴涵项覆盖，得最小 SOP；运动控制输出同法。

## 23. 蚁脑原理图（Ant Schematic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4c8bbf23e8daa24b235a3cad0f14b88e_Slide24.png" alt="Ant Schematic" width="80%"/>

各 SOP 用 AND/OR 直接实现，即得蚁脑原理图：几个 D 寄存器 + 一把逻辑门就能走出迷宫。

## 24. 一路都是 FSM？（FSMs All the Way Down?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a6b5790d1f81c2149319f5752970fb6d_Slide25.png" alt="FSMs All the Way Down" width="80%"/>

简单 FSM 可产生复杂群体行为（群集、鸟群、鱼群）；电影大战场面可想成大量并行 FSM。元胞自动机（通信 FSM 阵列）有时比解 PDE 更易建模分子约束。若允许 FSM 改自身转移表——或许可作进化的粗糙模型。FSM 无处不在。

## 25. 世界不跟我们的时钟跑！（The World Doesn’t Run on Our Clock!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b352897db8dac2d547ef777b639a9ac5_Slide26.png" alt="The World Doesn't Run on Our Clock" width="80%"/>

**异步输入**：跳变时刻与系统时钟完全无关（来自外界）。第 5 讲末：状态寄存器要求相对时钟上升沿满足建立/保持时间；随时可变的输入必可能违反。思路：对每个异步输入加**同步器**，使输出只在时钟上升沿后不久才改变。

## 26. 有界时间同步器（The Bounded-Time Synchronizer）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ed3ede13bf7e4e098ad023b0c54878b9_Slide27.png" alt="The Bounded-Time Synchronizer" width="80%"/>

规格：输入 IN、CLK，跳变时刻 $t_{\mathrm{IN}}$、$t_{\mathrm{C}}$。

- IN 充分早于 CLK → 在 CLK 后有界时间 $t_{\mathrm{D}}$ 内输出 1
- CLK 充分早于 IN → 在 $t_{\mathrm{D}}$ 内输出 0
- 两者间隔小于 $t_{\mathrm{E}}$ → $t_{\mathrm{D}}$ 内输出 0 或 1 皆可，但须是稳定的数字电平

结论：**对任意有限的 $t_{\mathrm{E}},t_{\mathrm{D}}$，即使元件 100% 可靠，也无法造出保证满足该规格的同步器**——问题不可解。

## 27. 不可解？那不可能…（Unsolvable? That Can’t Be True…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d71eb3e955c76f9e811471341b102aba_Slide28.png" alt="Unsolvable" width="80%"/>

能否用一个 D 寄存器当同步器？取 $t_{\mathrm{D}}$ = 寄存器传播延迟，$t_{\mathrm{E}}$ = setup/hold 中较大者。满足动态纪律时输出确定；违纪时规格允许 0 或 1——看似搞定。

陷阱：数字抽象诱使我们以为违纪后 $Q$ 在传播延迟后仍必为 0 或 1。看主锁存：当 B、C 几乎同时变时未必如此。

## 28. 神秘的亚稳态（The Mysterious Metastable State）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ae392b52efca848a9a3d845be454cf9f_Slide29.png" alt="The Mysterious Metastable State" width="80%"/>

主锁存是宽容 MUX，用正反馈构成双稳态。存储模式 ≈ 两门环路：约束为两门电路的 VTC（绿）以及 $V_{\mathrm{IN}}=V_{\mathrm{OUT}}$（红）。两曲线交于三点；中间交点是问题所在。

IN 与 CLK 同时变时，MUX 关闭启用反馈时 $Q$ 可能正处过渡，环路初值恰在或极近中间交点电压 → **亚稳态**（metastable state）：不稳定平衡。理论上可无限停留；微小扰动会推向稳定平衡。关键：**无法给系统停留在亚稳态的时间设上界**。

## 29. 亚稳态性质（Metastable State: Properties）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e46b265de4434246dc780c15ba83e737_Slide30.png" alt="Metastable State Properties" width="80%"/>

- 落在数字信令的禁止区 → **非法逻辑电平**；违纪后寄存器不再保证有界时间内给出数字输出
- **逻辑危害**：组合门对非法输入输出不可预测，可污染状态与输出
- **电气危害**：CMOS 输入在亚稳电压时 PFET/NFET 可同时导通，$V_{\mathrm{DD}}$–GND 直通，功耗尖峰
- 终将落到两稳定点之一；亚稳电压在 VTC 高增益区，一旦偏离会快速走向 0 或 $V_{\mathrm{DD}}$
- 分辨时间取决于初值离亚稳点有多近——无下界 → 分辨时间**无上界**；给定 $t_{\mathrm{D}}$，总有一段初值区间在时限内分不开
- 任一有限 $T$ 后仍亚稳的概率 $>0$；好消息：该概率随 $T$ **指数下降**
- 每个双稳态系统至少有一个亚稳态——正反馈存储的代价

更细的数学见课程笔记第 10 章。

## 30. 解法：延迟提高可靠性（Solution: Delay Increases Reliability）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/72c72ab553043ba0ce3ce2597ec0585c_Slide31.png" alt="Solution: Delay Increases Reliability" width="80%"/>

做法：D 寄存器同步后再加**第二级寄存器**，把可能亚稳的值**隔离（quarantine）**。第一级违纪亚稳时，第二级挡住；半周期内第二级主锁存关闭，完全不看该值。要到下一时钟边沿（整周期后）第二级才需要合法稳定输入。整周期后仍亚稳的概率可通过加长时钟周期压到任意低——不是 100% 保证，但失效间隔可达年/十年量级；无第二级可能每几小时就失败一次。

时钟周期短却要长隔离时间：可串多级隔离寄存器——决定失败概率的是“第一级亚稳 → 内部逻辑使用该信号”的总延迟。

**结论**：用同步寄存器隔离可能亚稳的信号；仍亚稳概率随隔离时间指数下降，可把失败率压到任意低。非绝对保证，但配合隔离策略时，亚稳态在实践中不再是问题。
