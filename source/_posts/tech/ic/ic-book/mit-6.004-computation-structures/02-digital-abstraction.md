---
title: MIT 6.004：L02 数字抽象
date: 2026-08-11 10:02:00
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L02 注解幻灯片。
>
> 源网页：[2.1 Annotated Slides | The Digital Abstraction](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c2/c2s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L02：数字抽象（The Digital Abstraction）

上一讲讨论了如何把信息编码成比特序列。本讲转向：为比特寻找有用的物理表示，这是构建信息处理器件的第一步。

## 1. 编码信息（Encoding Information）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dc16c05b0a93cd723b054e87ba1c3a79_Slide02.png" alt="Encoding Information" width="80%"/>

好的比特表示应具备哪些性质？

- **小且便宜**：随身携带数十亿比特（如音乐文件），网上还有海量比特可供访问，因此比特必须体积小、成本低。
- **长期稳定**：一旦是 0，就应长期保持为 0。罗塞塔石碑（Rosetta Stone，约公元前 196 年）近 2000 年后仍可辨读——但石刻稳定却难改写。
- **便于操作**：能快速访问、变换、组合、传输与存储所编码的信息。

自然界的启发：DNA 用腺嘌呤（Adenine）、胸腺嘧啶（Thymine）、鸟嘌呤（Guanine）、胞嘧啶（Cytosine）编码遗传信息，分子尺度满足“小”的要求，也有人研究用生命化学做大规模计算。但我们既不想提着黏糊糊的 DNA，也不想带着石凿——那么该用什么表示比特？

## 2. 电来救急（Electricity to the Rescue）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7d28a51dc0ed99e68a860d1c8ca04d18_Slide03.png" alt="Electricity to the Rescue" width="80%"/>

可用带电粒子相关的电现象表示信息：

- 电荷造成电势差 → **电压**（voltage）
- 电荷流动 → **电流**（current）
- 电磁场的相位与频率 → 无线通信的基础

本课程用**电压**表示比特。例如 $0\,\mathrm{V}$ 表示 0，$1\,\mathrm{V}$ 表示 1；比特序列可用多根线上的多路电压，或单线上随时间变化的电压序列。

电压表示的优点：市电与电池供应相对便宜可靠；百余年积累的工程知识使我们能造出极小、极低功耗的存取与处理电路——稳态下信息不变时，功耗可接近零。

挑战：电压易受环境电磁场影响；远距离传输需导线；改变线上电压需时间，由导线的电阻与电容决定 RC 时间常数（现代集成电路中很小，但非零）。这些问题有成熟工程对策。

## 3. 用电压表示信息（Representing Information with Voltage）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/de240baae461e8062880c14fc99d2aad_Slide04.png" alt="Representing Information with Voltage" width="80%"/>

考虑用电压表示黑白图像：每个 $(x,y)$ 点有强度（黑最弱、白最强）。可把强度映到电压，如 $0\,\mathrm{V}$ 为黑、$1\,\mathrm{V}$ 为白，中间强度用中间电压。

每个点含多少信息？取决于能区分多少强度（电压）。若能分辨任意小差异，则每点信息量理论上无穷；工程上可分辨的差异存在下界。

要用 $N$ 比特表示的信息量，需在 $0\,\mathrm{V}$–$1\,\mathrm{V}$ 内区分 $2^N$ 个电压。例如 $N=2$ 需区分四个电平（如 $0$、$1/3$、$2/3$、$1\,\mathrm{V}$），廉价电压表即可。$N$ 任意大在理论上可行，但微伏乃至纳伏级精度既昂贵又慢，热噪声等还会模糊“瞬时电压”的含义。

因此，电压编码能力受限于能否**可靠、快速**地区分某一时刻的电压。

## 4. 用电压编码一幅图（Using Voltages to Encode a Picture）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a124acf81d70d7f7e5d12840286bc84c_Slide05.png" alt="Using Voltages to Encode a Picture" width="80%"/>

按约定光栅顺序（如左→右、上→下）扫描图像，把强度转为电压，得到随时间变化的电压序列。早期电视即如此：画面编成在黑白表示之间变化的电压波形，并扩展电压范围以携带行同步、帧同步等**同步信号**（sync signals）。这种可取指定范围内任意值的波形称为**连续波形**（continuous waveform）。

## 5. 信息处理 = 计算（Information Processing = Computation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/19c14a2a5ac2c91c00a04cc7784c6acf_Slide06.png" alt="Information Processing = Computation" width="80%"/>

用两个简单处理模块搭系统：

- **COPY**：输出复现输入电压 → 图像不变
- **INVERTING**：输入为 $V$ 时输出 $1-V$ → 黑白反转

使用预封装模块是构建大电路的常见方式：按规则连接即可，无需理解每个模块内部细节；不同配置下也可根据模块行为预测系统行为——像搭积木一样组装，即使不熟悉模拟电路细节的程序员也能搭建处理任务。前提是：组件正确、连接遵守规则时，系统行为应可预期。

## 6. 动手搭一个系统！（Let's Build a System!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/0be86c2cec235cbad710c685f8229ef2_Slide07.png" alt="Let's Build a System" width="80%"/>

用若干 COPY 与 INVERTING 模块组成图像处理系统。COPY 不改图像，INVERTING 为偶数个，理论上输出应与输入相同。

实际上输出略糊：强度有偏差，剧烈变化被抹平。出了什么问题？

## 7. 系统为何失败？（Why Did Our System Fail?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2cd7c04bd2da68b25b1839d1b842ea2e_Slide08.png" alt="Why Did Our System Fail" width="80%"/>

COPY / INVERTING 很难严格服从数学描述：制造偏差与环境差异使 COPY 对 $V$ 输入输出 $V+\varepsilon$，INVERTING 同理。在**连续值**强度表示下，$V+\varepsilon$ 仍是合法输出——只是对应另一幅略不同的合法图像。我们无法区分“略受损的信号”与“略不同图像的完美信号”。

更致命的是：**误差会沿链路累积**。系统越大，累积误差越大。若必须限制“还能做多少次计算才结果不可用”，系统将很难扩展。

噪声与不精确不可避免；无法可靠复现无限信息。必须设计系统在允许一定误差时仍能可靠处理信息——要能察觉处理引入的误差，并在误差累积前恢复正确值。这就是下一主题。

## 8. 数字抽象（The Digital Abstraction）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b5d88bbde4ed3b8c9a7e85f871aa6f66_Slide09.png" alt="The Digital Abstraction" width="80%"/>

引入**数字抽象**（digital abstraction）：用连续的电压世界表示一个小的有限值集合——此处即二进制的 0 与 1。世界本身并非天生数字；我们是用连续物理现象去工程出数字行为。

旁注：有些物理量天然离散（如电子自旋）；量子计算正研究如何利用量子物理做计算。本课程聚焦于如何用经典连续现象构建数字系统。

## 9. 用电压做数字表示（Using Voltages Digitally）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b4e062668dc8fe88e3a04ed4fd5dabbf_Slide10.png" alt="Using Voltages Digitally" width="80%"/>

核心：约定信令，每次只编码 **1 比特**（0 或 1），系统中各组件与导线使用统一表示。到达可用方案需三轮尝试。

**第一版**：用阈值 $V_{\mathrm{th}}$ 把电压范围一分为二——$V < V_{\mathrm{th}}$ 为 0，$V \ge V_{\mathrm{th}}$ 为 1。数学上简洁，但阈值附近的电压极难可靠判读：电路需精密元件与严格受控环境，与低成本、多环境使用目标不符。→ **不可行（大红叉）**。

**第二版**：引入两个阈值 $V_{\mathrm{L}}$、$V_{\mathrm{H}}$：

- $V \le V_{\mathrm{L}}$ → 解释为 0
- $V \ge V_{\mathrm{H}}$ → 解释为 1
- $V_{\mathrm{L}}$ 与 $V_{\mathrm{H}}$ 之间为**禁区**（forbidden zone）：系统可将该区电压解为 0 或 1，不必一致，甚至可不给出解释

这样可用高增益运放加禁区内粗略参考电压做“快而糙”的电压→比特转换；参考不必极准（如 10% 精度电阻分压），温度与电源漂移也可容忍——只需保证在 $V_{\mathrm{L}}$ 以下或 $V_{\mathrm{H}}$ 以上时行为正确。→ **暂给绿勾**；稍后再做一次小修正。

## 10. 组合器件（Combinational Devices）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/bc62e73670f544e772dee970def1402b_Slide11.png" alt="Combinational Devices" width="80%"/>

满足以下四条的器件称为**组合器件**（combinational device）：

1. **数字输入**：按信令约定，$V \le V_{\mathrm{L}}$ 为 0，$V \ge V_{\mathrm{H}}$ 为 1
2. **数字输出**：输出 0 时电压 $\le V_{\mathrm{L}}$，输出 1 时电压 $\ge V_{\mathrm{H}}$
3. **功能规格**：对每种可能的数字输入组合，规定各输出的值（例：三输入有 $2^3=8$ 种组合，可用 8 行真值表）
4. **时序规格**：至少给出**传播延迟**（propagation delay）$t_{\mathrm{PD}}$——从输入到达稳定有效数字值，到输出保证稳定有效的时间**上界**

合称**静态纪律**（static discipline），所有组合器件必须遵守。

## 11. 组合数字系统（A Combinational Digital System）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/90fb5993efed87da9f29f21d6521df65_Slide12.png" alt="A Combinational Digital System" width="80%"/>

由组合组件组成更大组合系统的规则：

1. 每个组件本身是组合器件
2. 每个组件的每个输入：接系统输入、恰好接另一器件的一个输出，或接表示 0/1 的恒定电压
3. 互连**无有向环**——从系统输入到输出的路径中，任一组件最多出现一次

主张：按此规则构建的系统本身也是组合器件；系统可任意大，仍服从静态纪律（不同于本节开头脆弱的模拟链路）。

## 12. 这是组合器件吗？（Is This a Combinational Device?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1952027a839b7ca97ff3bdcffa8385b6_Slide13.png" alt="Is This a Combinational Device" width="80%"/>

以由组合器件 A、B、C 组成的系统为例，验证整体是否服从静态纪律：

1. **数字输入？** 是——系统输入即某些组件的输入，组件组合 ⇒ 系统输入数字。
2. **数字输出？** 是——同理由组件继承。
3. **功能规格？** 可——无环，按拓扑顺序用各组件功能规格逐步求出内部信号与输出。
4. **$t_{\mathrm{PD}}$？** 可——枚举输入到输出的有限路径，路径延迟为沿途各组件 $t_{\mathrm{PD}}$ 之和；系统 $t_{\mathrm{PD}}$ 取所有路径中的**最大值**（最长路径）。

因此整体是组合器件。可用组合规则构建任意复杂度的组合器件。

## 13. 应对噪声（Dealing With Noise）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4e0f1639c2b25a04e37458e7d6897668_Slide14.png" alt="Dealing With Noise" width="80%"/>

定稿信令前还有问题：上游器件输出略低于 $V_{\mathrm{L}}$ 的合法 0；线上噪声使下游看到略高于 $V_{\mathrm{L}}$ 的电压——不再是有效数字输入，下游组合行为不再有保证。

对策：让**输出约束严于输入**——合法输出可叠加一定噪声后，仍落在合法输入范围内。能否靠“消灭噪声”一劳永逸？用电学组件时做不到。

## 14. 噪声从哪来？（Where Does Noise Come From?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/566c1b2f734934277c5d7b038f6cb906_Slide15.png" alt="Where Does Noise Come From" width="80%"/>

**电压噪声**（偏离标称电压）来源包括：

- **电学效应**：导体 IR 压降（欧姆定律）、导体间电容耦合、引线电感与变电流引起的 $L(\mathrm{d}I/\mathrm{d}t)$ 等
- **制造偏差**：器件参数相对标称值的偏差导致器件间电学行为差异
- **环境因素**：热噪声、外部电磁场等

许多噪声来自电路正常工作或材料/工艺固有性质，无法消除；但可估计幅度并相应调整信令规格。

## 15. 噪声容限（Noise Margins）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a104c51dda65b0f36c3fbe4d4c000375_Slide16.png" alt="Noise Margins" width="80%"/>

最终信令：输入与输出使用**不同**阈值。

**输出**：

- 送出 0：电压 $\le V_{\mathrm{OL}}$
- 送出 1：电压 $\ge V_{\mathrm{OH}}$

**输入**：

- 电压 $\le V_{\mathrm{IL}}$ → 解释为 0
- 电压 $\ge V_{\mathrm{IH}}$ → 解释为 1

约束：$V_{\mathrm{IL}}$ **严格大于** $V_{\mathrm{OL}}$，$V_{\mathrm{IH}}$ **严格小于** $V_{\mathrm{OH}}$。输入与输出阈值之间的间隙称为**噪声容限**（noise margins）：

- 低噪声容限：$V_{\mathrm{IL}} - V_{\mathrm{OL}}$
- 高噪声容限：$V_{\mathrm{OH}} - V_{\mathrm{IH}}$

二者中较小者称为该信令规格的**噪声免疫**（noise immunity）。服从该规格的组合器件会在误差累积前“清洗”输入噪声——数字信令不再重蹈前文模拟例子的覆辙。

## 16. 电压传输特性（Voltage Transfer Characteristic）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c0159bc28d5b9890e4ea9fc534d7312b_Slide17.png" alt="Voltage Transfer Characteristic" width="80%"/>

以最简单的组合器件——**缓冲器**（buffer，单输入单输出，输出经传播延迟后复现输入数字值）为例测量。它服从静态纪律，并使用含高低噪声容限的修订信令。

测量：将输入设为从 $0\,\mathrm{V}$ 到电源电压的一系列值，每次等待输出稳定（即等满 $t_{\mathrm{PD}}$），在横轴 $V_{\mathrm{IN}}$、纵轴 $V_{\mathrm{OUT}}$ 上描点，得到**电压传输特性**（voltage transfer characteristic, **VTC**）。

静态纪律约束合法组合器件的 VTC：有效输入必须产生有效输出（“valid in, valid out”）。图上可标出**禁区**——有效数字输入却对应无效数字输出；合法器件的 VTC 不得落入这些区域。缓冲器的实测曲线（黑线）不穿过阴影区，符合要求。这些测量刻画的是**静态**行为，不直接给出速度。

## 17. VTC 推论（VTC Deductions）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a9fd8ad2231bc0c72f981f044d3917db_Slide18.png" alt="VTC Deductions" width="80%"/>

关于 VTC 的两点观察：

1. 中心白色区域对应输入在 $V_{\mathrm{IL}}$–$V_{\mathrm{IH}}$（禁区）。静态纪律只约束**有效**输入，禁区输入下输出可任意。
2. 由正噪声容限，$V_{\mathrm{OH}} - V_{\mathrm{OL}}$ **严格大于** $V_{\mathrm{IH}} - V_{\mathrm{IL}}$，故中心白区**高大于宽**。穿过该区的曲线必有一段斜率绝对值 **大于 1**——输入小变化引起输出更大变化，即器件**增益**（gain）$>1$ 或 $<-1$。

若组件可互连，则 $V_{\mathrm{IN}}$ 与 $V_{\mathrm{OUT}}$ 范围相同，VTC 图画在正方形内。因存在 $|\text{斜率}|>1$ 的区段，又不能全程 $|\text{斜率}|>1$，曲线斜率必须改变——这种器件称为**非线性器件**（nonlinear devices）。

结论：仅用电阻、电容、电感等**线性器件**无法构成组合器件；需要增益 $>1$ 的非线性器件。寻找这类器件是下一讲主题。

## 18. VTC 实例（VTC Example）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7d3e2ce5bee51d34132f9255cdc0d956_Slide19.png" alt="VTC Example" width="80%"/>

给定某器件的 VTC，能否选 $V_{\mathrm{OL}}$、$V_{\mathrm{IL}}$、$V_{\mathrm{IH}}$、$V_{\mathrm{OH}}$ 使其成为合法组合反相器？

反相器：数字 0 入 → 数字 1 出，反之亦然。该器件低输入时输出高，有希望。

选取示例：

- 器件最低输出约 $0.5\,\mathrm{V}$ ⇒ 取 $V_{\mathrm{OL}} \ge 0.5\,\mathrm{V}$（取 $0.5\,\mathrm{V}$）
- 输入高于约 $3\,\mathrm{V}$ 时输出 $\le V_{\mathrm{OL}}$ ⇒ 取 $V_{\mathrm{IH}} = 3\,\mathrm{V}$（尽量低以留高噪声容限）
- 设噪声容限 $N = 0.5\,\mathrm{V}$，则 $V_{\mathrm{IL}} = V_{\mathrm{OL}} + N = 1\,\mathrm{V}$，$V_{\mathrm{OH}} = V_{\mathrm{IH}} + N = 3.5\,\mathrm{V}$

在图上标出阈值与禁区后，VTC 合法。因此可用规格 $V_{\mathrm{OL}}=0.5\,\mathrm{V}$、$V_{\mathrm{IL}}=1\,\mathrm{V}$、$V_{\mathrm{IH}}=3\,\mathrm{V}$、$V_{\mathrm{OH}}=3.5\,\mathrm{V}$ 把该器件当作组合反相器。

## 19. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b2386ed22dcaa360690f7bb92ceccd46_Slide20.png" alt="Summary" width="80%"/>

本讲要点：

- 用电压表示信息可行，但**连续模拟**链路中误差会累积，难以扩展
- **数字抽象**：用连续电压只表示有限符号（0/1），并引入禁区与统一信令
- **组合器件**服从静态纪律：数字 I/O、功能规格、传播延迟 $t_{\mathrm{PD}}$
- 组合组件按规则（无环等）互连 → 更大组合系统
- 输入/输出分离阈值 → **噪声容限**，抗噪声累积
- 合法组合器件的 VTC 避开禁区，且必含增益 $>1$ 的非线性区段；下一讲寻找可用的物理器件（CMOS）
