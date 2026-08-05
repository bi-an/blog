---
title: CMOS
date: 2026-08-04 15:57:13
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L03 注解幻灯片。
>
> 源网页：[3.1 Annotated Slides | CMOS](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c3/c3s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L03：CMOS 技术（CMOS Technology）

## 1. 组合器件愿望清单（Combinational Device Wish List）

![Combinational Device Wish List](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/42e5019be6a79c380e6a8cb9833f028d_Slide02.png)

回顾组合器件（combinational device）应具备的特性。上一讲用电压表示信息，并允许信息在处理元件系统中流动时承受一定误差。

规定了四个信号阈值（signaling thresholds）：

- $V_{\mathrm{OL}}$、$V_{\mathrm{OH}}$：分别是组合器件输出端表示 0、1 的电压上/下界
- $V_{\mathrm{IL}}$、$V_{\mathrm{IH}}$：分别用于解释组合器件输入端电压时的对应角色

并要求 $V_{\mathrm{OL}}$ 严格小于 $V_{\mathrm{IL}}$，二者之差称为**低噪声容限**（low noise margin）——输出信号可叠加的噪声量，仍能在相连输入端被正确解释。同理要求 $V_{\mathrm{IH}}$ 严格小于 $V_{\mathrm{OH}}$。

在**电压传输特性**（voltage transfer characteristic, VTC）——即 $V_{\mathrm{OUT}}$ 对 $V_{\mathrm{IN}}$ 的曲线——中，这些阈值意味着：稳态下有效输入必须产生有效输出，因此 VTC 中存在**禁区**（forbidden regions）。合法组合器件的 VTC 不能落入这些区域。由四个阈值围成的中心区域“宽度小于高度”，因此合法 VTC 必须有一段**增益大于 1**，且整体**非线性**。图中所示即为反相器（inverter）的 VTC。

若电路技术增益高、输出电压贴近地与电源轨，可将 $V_{\mathrm{OL}}$、$V_{\mathrm{OH}}$ 向外推向电源轨，将 $V_{\mathrm{IL}}$、$V_{\mathrm{IH}}$ 向内收，从而增大噪声容限。

此外还希望：

- 器件便宜且小（数字系统需要数十亿器件）
- 空闲且内部电压不变时尽量**零功耗**（zero power dissipation）
- 能实现有用逻辑功能的器件目录

满足上述愿望的技术，正是本讲主题。

## 2. N 沟道 MOSFET：物理视角（N-Channel MOSFET: Physical View）

![N-Channel MOSFET Physical View](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d18fcd081f55667ad0111c80b6173df6_Slide03.png)

主角是**金属–氧化物–半导体场效应晶体管**（metal-oxide-semiconductor field-effect transistor, **MOSFET**）。

图为 MOSFET 三维剖面：由多层电学材料构成的“三明治”，作为**集成电路**（integrated circuit, IC）的一部分，在同一套制造步骤中批量制成。

现代工艺中，图示块边长约数十纳米——约为细发丝厚度的 1/1000。尺寸小于可见光波长（约 400–750 nm），普通光学显微镜无法分辨。工程师约每 24 个月将特征尺寸缩小一半左右，即 **摩尔定律**（Moore's Law，Gordon Moore，1965）。尺寸缩小 50%，同面积可集成约 4 倍器件，器件本身也更快。1975 年一块 IC 或约有 2500 个器件；今日可达二三十亿。

结构要点：

- **衬底**（substrate）：硅晶圆掺入杂质导电。图中掺入**受主**（acceptor）如硼（Boron），形成 **P 型半导体**（p-type semiconductor）。IC 对 P 型衬底有一处**电接触**（electrical contact），该端称为 **bulk 端**（bulk terminal），用于控制衬底电压。
- **绝缘**：用二氧化硅（SiO₂）。隔离栅极（gate，图中红色）与衬底的氧化层极薄，使栅上电荷的电场易于影响下方衬底。
- **栅极**（gate）：导体，此处为多晶硅（polycrystalline silicon）[^poly-gate]。栅、薄氧与 P 型衬底构成电容；改变栅压会改变栅下衬底的电学状态。早期栅极为金属，“金属–氧化物–半导体”（metal-oxide-semiconductor, MOS）即指该结构。
- 栅极做好后，在栅两侧矩形区注入**施主**（donor）如磷（Phosphorous），形成 **N 型**（n-type）区，即 MOSFET 另两端：**源极**（source）与**漏极**（drain）。二者物理上通常相同，按工作中的角色区分。

MOSFET 可视为连接源、漏的**压控开关**（voltage-controlled switch）。导通时，电流经栅电容“下极板”形成的导电沟道从漏流向源。关键尺寸：

- **沟道长度** L（channel length）：漏到源电流需跨越的距离
- **沟道宽度** W（channel width）：可供导电的沟道多少

漏–源电流 $I_{\mathrm{DS}}$ 与 W/L 成正比。设计常取尽量短的 L（新闻中“14 nm 工艺”的 14 nm 多指最小允许沟道长度），用 W 设定所需电流。$I_{\mathrm{DS}}$ 大则源/漏节点电压翻转快，但器件物理更大。

小结：MOSFET 有四个电学端子——bulk、gate、source、drain；设计者可控尺寸为沟道长度（通常尽量小）与沟道宽度（设定电流）。它是固态开关，无机械运动，由四端相对电压决定的电场控制。

## 3. N 沟道 MOSFET：电学视角（N-Channel MOSFET: Electrical View）

![N-Channel MOSFET Electrical View](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8ab313bffd2125bf69badbb56dc95c26_Slide04.png)

约定：两侧扩散区中**电位较高**的称为漏（drain），较低的称为源（source）；若有电流，则从漏流向源。

器件有阈值电压（threshold voltage）$V_{\mathrm{TH}}$：开关从**断路**（non-conducting / OFF / OPEN）变为**导通**（conducting / ON / CLOSED，即路径闭合）的分界。现代工艺中 N 沟道 MOSFET 的 $V_{\mathrm{TH}}$ 约 0.5 V。

图左侧的 P+ **端**（terminal）即通往 P 型衬底的**连接**（connection）。正常工作时，衬底电压须始终 ≤ 源、漏电压。

开关由栅源电压差控制：$V_{\mathrm{GS}} = V_G - V_S$。

- 当 $V_{\mathrm{GS}} < V_{\mathrm{TH}}$：开关开路，源漏无电连接。N 型与 P 型材料**物理接触**（physical contact）处形成**耗尽区**（depletion region，图中深红），载流子迁离结区，起到绝缘作用；源/漏相对衬底电压越高，该绝缘层越宽，并填满源漏之间，使二者电隔离。
- 当 $V_{\mathrm{GS}}$ 增大：栅上正电荷电场吸引衬底电子。达到阈值后，电子从价带进入导带，聚集在薄氧化层下，足够时半导体由 P 型反转为 N 型，形成连接源漏的**反型层**（inversion layer）。开关闭合；电流与漏源电压差 $V_{\mathrm{DS}}$ 成比例。此时反型层近似电阻，遵循欧姆定律 $I_{\mathrm{DS}} = V_{\mathrm{DS}}/R$。过程可逆：$V_{\mathrm{GS}}$ 低于阈值则反型层消失，开关截止。
- 当 $V_{\mathrm{DS}} > V_{\mathrm{GS}}$：沟道电场几何改变，漏端附近反型层**夹断**（pinch-off）；电子仍可隧穿夹断点到达源侧导电沟道。对 $I_{\mathrm{DS}}$ 的影响见下一节曲线。

## 4. N 沟道 MOSFET：$I_{\mathrm{DS}}$ 与 $V_{\mathrm{DS}}$

![N-Channel MOSFET IDS vs VDS](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/05e66c788f5ea8de4dafcc2f00668c80_Slide05.png)

每条曲线是固定 $V_{\mathrm{GS}}$ 下 $I_{\mathrm{DS}}$ 对 $V_{\mathrm{DS}}$ 的关系。

- $V_{\mathrm{GS}} \le V_{\mathrm{TH}}$ 时 $I_{\mathrm{DS}} = 0$（前几条曲线叠在横轴上）
- 超过阈值后，$I_{\mathrm{DS}}$ 随 $V_{\mathrm{GS}}$ 增大而增大（反型层更厚，导电更强）
- $V_{\mathrm{DS}}$ 较小时，器件呈电阻特性：曲线左侧线性段斜率与沟道电阻成反比；$V_{\mathrm{GS}}$ 越大斜率越陡、电阻越小
- $V_{\mathrm{DS}}$ 较大时沟道在漏端夹断，$I_{\mathrm{DS}}$ 近似不再随 $V_{\mathrm{DS}}$ 增加，曲线近乎水平——进入**饱和区**（saturation）

饱和段并非完全水平，$I_{\mathrm{DS}}$ 仍略随 $V_{\mathrm{DS}}$ 上升，称为**沟道长度调制**（channel-length modulation）。

对设计者而言，遵守简单规则后，可用压控开关的简化心智模型，而不必每次展开上述复杂物理。

## 5. 两种 FET（FETs Come in Two Flavors）

![FETs Come in Two Flavors](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b96ef74b7191d7adb37c08c33f7955ec_Slide06.png)

此前讨论的是：P 型衬底上的 N 型源/漏扩散——**N 沟道 MOSFET**（n-channel MOSFET），反型层为 N 型。电路中常将 bulk 接 GND，保证 P 型衬底电压 ≤ 源漏。

对调所有材料类型：N 型衬底上的 P 型源/漏——**P 沟道 MOSFET**（p-channel MOSFET）。同样是压控开关，但电位关系相反：使 N 沟道导通的控制电压往往使 P 沟道截止，反之亦然。

两类开关行为互补，故使用二者的电路称为**互补 MOS**（complementary MOS, **CMOS**）。

## 6. CMOS 配方（CMOS Recipe）

![CMOS Recipe](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ae0e5e40b7aed5171cf0f5d264d37700_Slide07.png)

用 MOSFET 处理数字编码信息时，遵守两条规则，即可把 MOSFET 抽象为压控开关：

1. **N 沟道 MOSFET（NFET）**仅用于**下拉电路**（pulldown），把信号节点接到电源地（GND）。下拉导通时节点约 0 V，记为数字 0。心智模型：栅为数字 0 → 关；栅为数字 1 → 开。$V_{\mathrm{GS}}$ 越大，有效电阻越小，$I_{\mathrm{DS}}$ 越大。
2. **P 沟道 MOSFET（PFET）**仅用于**上拉电路**（pullup），把信号节点接到电源电压 $V_{\mathrm{DD}}$。上拉导通时节点为 $V_{\mathrm{DD}}$，记为数字 1。PFET 阈值为负，$V_{\mathrm{GS}}$ 须小于阈值才导通。心智模型与 NFET **相反**：栅为 0 → 开；栅为 1 → 关。

为何不能用 NFET 做上拉、PFET 做下拉？简答：信号电平会退化，噪声容限受损（实验课会展开）。

## 7. CMOS 反相器 VTC（CMOS Inverter VTC）

![CMOS Inverter VTC](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/800a39e762facfc0efb025112e8d1617_Slide08.png)

CMOS 反相器：输入 0 → 输出 1，反之亦然。电路由一个 NFET 下拉（输出到 GND）与一个 PFET 上拉（输出到 $V_{\mathrm{DD}}$）组成，两管栅极同接输入。

- 输入为数字 0：NFET 关、PFET 开，输出充至 $V_{\mathrm{DD}}$（数字 1）；源漏同为 $V_{\mathrm{DD}}$ 后无压差，电流停止
- 输入为数字 1：NFET 开、PFET 关，输出放至 0 V；达 0 V 后电流停止
- 输入处于中间电平时，视电源与阈值，上拉与下拉可能短暂同时导通。此时输入小变化可引起输出大变化，形成 CMOS 的高增益，从而可选取宽松噪声容限

这是第一个 CMOS 组合逻辑门（combinational logic gate）。

## 8. 超越反相器（Beyond Inverters）

![Beyond Inverters](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fb14c846b7a09e4e73aee2119a5fa49d_Slide09.png)

构建其他逻辑门：设计**互补**的上拉与下拉网络（complementary pullup / pulldown）。“互补”指一个导通时另一个不导通。

- 上拉导通、下拉截止 → 输出接 $V_{\mathrm{DD}}$ → 数字 1
- 下拉导通、上拉截止 → 输出接 GND → 数字 0
- 二者长时间同时导通 → $V_{\mathrm{DD}}$ 到 GND 有大短路电流；简单开关模型下输出记为 **X（未知）**（unknown）
- 二者都不导通 → 输出**浮空**（floating），节点电容上的电荷可暂存——这是一种存储形式，后续讲座再谈

目前只关注互补上拉/下拉的器件。

## 9. CMOS 互补结构（CMOS Complements）

![CMOS Complements](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/746491e8a34a989bc55593c6ce32dde0_Slide10.png)

最简单互补对：单个 NFET 下拉 + 单个 PFET 上拉，同一信号控制时，一开则另一必关。

- **串联 NFET**：仅当 A=1 且 B=1 时整条路径导通。其互补是**并联 PFET**：A=0 或 B=0 时导通。对 AB = 00, 01, 10, 11 逐一验证可知总有一个网络导通、另一个截止。
- **并联 NFET** 与 **串联 PFET** 同理互补。

## 10. 小测验：NAND 门（A Pop Quiz!）

![NAND Gate](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/87d30b72542f201ec57b1b55586de37a_Slide11.png)

串联 NFET 下拉 + 并联 PFET 上拉。用**真值表**（truth table）列出所有 A、B 组合下的 Z：

| A | B | Z |
|---|---|---|
| 0 | 0 | 1 |
| 0 | 1 | 1 |
| 1 | 0 | 1 |
| 1 | 1 | 0 |

仅当 A、B 皆为 1 时 Z=0，否则 Z=1——即 **NAND**（NOT-AND）门。

左图为俯视版图示意：蓝为金属（上下大走线接 $V_{\mathrm{DD}}$ / GND），红为多晶硅栅，绿为 NFET 的 N 型扩散，棕黄为 PFET 的 P 型扩散。可看出 NFET 串联、PFET 并联。

成本直觉（讲义信封估算）：一块 300 mm 晶圆上约可做 260 亿个 NAND；旧工艺材料与制造约 \$3500，约合每门百纳美元量级——确属又小又便宜。

## 11. 通用 CMOS 门配方（General CMOS Gate Recipe）

![General CMOS Gate Recipe](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/395c88f9208e5d84e701893734adab97_Slide12.png)

更复杂逻辑：用串联/并联开关网络实现。

设计步骤：

1. 先设计 PFET 上拉：输出为 1 的输入条件如何接到 $V_{\mathrm{DD}}$。例：F=1 当 A=0 **或**（B=0 **且** C=0）。OR → 并联，AND → 串联。
2. 构造互补下拉：沿上拉层次结构，PFET→NFET，串联↔并联。例：上拉为“A 控制管并联于 B–C 串联”；下拉则为“A 控制管串联于 B–C 并联”，且全部为 NFET。
3. 合并上拉与下拉，得到全互补 CMOS 实现。

该配方并非对一切逻辑函数都适用，见下一节。

## 12. CMOS 门天然反相（CMOS Gates Are Naturally Inverting）

![CMOS Gates Are Naturally Inverting](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/69620041d8bfc75f7440f53d56a04852_Slide13.png)

单个 CMOS 门（一个上拉网络 + 一个下拉网络）只能实现**反相函数**（inverting functions）：输入上升导致输出下降，反之亦然。

理由：某输入从 0→1 时，受其控制的 NFET 由关→开（可能接通到 GND 的路径），PFET 由开→关（可能切断到 $V_{\mathrm{DD}}$ 的路径）。若输出因此变化，必是下拉被启用、上拉被禁用，即输出从 1→0。同理，输入下降对应输出上升。

对非常数 CMOS 门：全部输入为 0 时输出必为 1（全部 NFET 关、全部 PFET 开）；全部输入为 1 时输出必为 0。因此**正逻辑**（positive logic，如 AND）不能用单个 CMOS 门实现——AND 真值表在全 0 / 全 1 时与上述结论矛盾，且 A=1、B 从 0→1 时输出上升而非下降。CMOS 设计者需熟练用反相逻辑拼出所需功能。

## 13. CMOS 时序规格（CMOS Timing Specifications）

![CMOS Timing Specifications](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/127417502caff69dbaa0b1e9764ffad3_Slide14.png)

两级反相器串联，考察左级翻转时发生什么。$V_{\mathrm{IN}}$ 从 0→1：上拉 PFET 关、下拉 NFET 开，左级输出接到 GND。

电学模型含：连接左右的导线分布电阻/电容，以及右级 MOSFET 栅端电容。电荷经导线与 NFET 沟道电阻泄放到 GND，电压指数趋近 0 V。$V_{\mathrm{IN}}$ 下降时输出向 $V_{\mathrm{DD}}$ 充电，过程类似。

## 14. 传播延迟（Propagation Delay）

![Propagation Delay](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/32605907e13d9fe3f9daf48e21474a70_Slide15.png)

输入/输出波形均非瞬时，如何量延迟？用信号阈值定义。

**传播延迟**（propagation delay）$t_{\mathrm{PD}}$：从有效输入到有效输出的延迟**上界**。有效输入由 $V_{\mathrm{IL}}$、$V_{\mathrm{IH}}$ 界定；有效输出由 $V_{\mathrm{OL}}$、$V_{\mathrm{OH}}$ 界定。

- 上升输入：从 $V_{\mathrm{IN}}$ 越过 $V_{\mathrm{IH}}$，到 $V_{\mathrm{OUT}}$ 越过 $V_{\mathrm{OL}}$ 的时间间隔
- 下降输入：从 $V_{\mathrm{IN}}$ 越过 $V_{\mathrm{IL}}$，到 $V_{\mathrm{OUT}}$ 越过 $V_{\mathrm{OH}}$ 的时间间隔

$t_{\mathrm{PD}}$ 须 ≥ 上述任意测得延迟；厂商还需覆盖工艺、温度、电源等变化，使客户实测延迟不超过该规格。

设计者可用各组件的 $t_{\mathrm{PD}}$ 估算系统延迟。要减小延迟，需减小电阻与电容：加宽 MOSFET 可降有效电阻，但增大栅电容、拖慢驱动该栅的前级——这是晶体管尺寸优化问题。

## 15. 污染延迟（Contamination Delay）

![Contamination Delay](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9e113bf15832689f78707177f411b52f_Slide16.png)

**污染延迟**（contamination delay）$t_{\mathrm{CD}}$：输入开始变化、变为无效之后，输出仍保持原先有效值的时间。技术上是从无效输入到无效输出的延迟**下界**。

- 上升输入：从 $V_{\mathrm{IN}}$ 越过 $V_{\mathrm{IL}}$（不再是有效 0），到 $V_{\mathrm{OUT}}$ 越过 $V_{\mathrm{OH}}$（不再是有效 1）
- 下降输入：做对称测量

$t_{\mathrm{CD}}$ 须 ≤ 任意测得该间隔。静态纪律并不强制要求 $t_{\mathrm{CD}}$；未给出时，设计者应保守取 $t_{\mathrm{CD}} = 0$（输入一无效，输出即可立即无效）。厂商常称其为“最小传播延迟”（minimum propagation delay）。

## 16. 组合契约（The Combinational Contract）

![The Combinational Contract](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/039976353830d78cae70762b3f946063_Slide17.png)

时序规格小结（输入波形 A，输出波形 B）：

- $t_{\mathrm{CD}}$：旧输出值保持有效的**最短**时间下界；保守可取 0，表示输入一变输出即可变
- $t_{\mathrm{PD}}$：输出重新有效并稳定的**最长**时间上界

一般而言，在输入跳变后的 $(t_{\mathrm{CD}}, t_{\mathrm{PD}})$ 区间内，对输出行为**不作保证**：可多次翻转，也可出现非数字电压。后文对一类特殊组合器件可再说得更细；通常设计者不应对该区间内 B 的值做假设。

## 17. 无环组合电路（Acyclic Combinational Circuits）

![Acyclic Combinational Circuits](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ef539cae30e79f7e1ae926a1084faf55_Slide18.png)

由组件规格推大电路规格的例子：四个 NAND，每个 $t_{\mathrm{PD}} = 4\,\mathrm{ns}$，$t_{\mathrm{CD}} = 1\,\mathrm{ns}$。

- **整体 $t_{\mathrm{PD}}$**：枚举从输入 A/B/C 到输出 Y 的每条路径，累加路径上各门 $t_{\mathrm{PD}}$，取**最大**。例中最长路径经 3 个 NAND → $12\,\mathrm{ns}$（Y 保证在输入跳变后 12 ns 内稳定有效）
- **整体 $t_{\mathrm{CD}}$**：同样枚举路径，累加各门 $t_{\mathrm{CD}}$，取**最小**。例中最短路径经 2 个 NAND → $2\,\mathrm{ns}$（输入无效后 Y 至少再保持旧值 2 ns）

## 18. 最后一个时序问题（One Last Timing Issue）

![One Last Timing Issue](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/028c371b2c5c10a44a097b088b643fcf_Slide19.png)

非 CMOS 的 NOR 组合器件：A、B 初为 0，Z=1；B：0→1 后 Z 最终 1→0，跳变落在 $t_{\mathrm{CD}}$–$t_{\mathrm{PD}}$ 窗口内（图中红阴影表示该区间无保证）。

另一情形：A、B 初为 1，Z=0。真值表显示 A=1 时 Z 恒为 0，与 B 无关。B：1→0 后，经 $t_{\mathrm{PD}}$ 后 Z 仍应为 0；但**一般**在中间区间仍不能假设 Z 的行为——合法组合器件可在该窗口任意表现。

许多工艺（如 CMOS）遵守更严的约束。

## 19. 宽容门（Lenient Gates）

![Lenient Gates](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f9fc1b89b777631c213831b6d134866f_Slide20.png)

CMOS NOR：两输入皆为 1 时，NFET 导通、PFET 截止，Z 接 GND。B：1→0 时 B 控制的开关翻转，但上拉仍不导通，Z 到 GND 仍有路径——只是路径从两条减为一条，Z **全程**保持有效稳定的 0。即：一输入为 1 时，另一输入跳变不影响输出有效性。

**宽容组合器件**（lenient combinational device）：只要足以决定输出的那组输入已有效至少 $t_{\mathrm{PD}}$，输出就保证有效；触发该行为时，其余输入的跳变不影响输出有效性。多数 CMOS 逻辑门天然宽容。

真值表可用 X 标出无关输入：宽容 NOR 中，A=1 时 B 无关，B=1 时 A 无关；无关输入的跳变不触发通常的 $t_{\mathrm{CD}}$ / $t_{\mathrm{PD}}$ 输出时序。构建存储元件时会需要宽容组件。

## 20. 小结（Summary）

![Summary](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a5969ad6a91e420d2418dba308c89306_Slide21.png)

本讲要点：

- MOSFET 作为压控开关；N/P 沟道互补构成 CMOS
- 规则：NFET 只做下拉，PFET 只做上拉；上拉与下拉网络互补
- 单个 CMOS 门天然实现反相逻辑；复杂函数由串并联网络按配方设计
- 时序用 $t_{\mathrm{PD}}$（上界）与 $t_{\mathrm{CD}}$（下界，可保守取 0）刻画；大电路取路径最大/最小累加
- CMOS 门常具宽容性，利于后续存储等设计

可开始自行搭建 CMOS 门了。

[^poly-gate]: **备注：为什么栅极用多晶硅？**

    1. **为什么多晶硅能当导体？**  
       本征多晶硅导电很差；做成栅极时会**重掺杂**（heavily doped），载流子浓度极高，近似金属般导电。工艺上常再叠一层**硅化物**（silicide，如 Ti/Co/Ni 硅化物，自对准时常称 **salicide**），进一步降低栅电阻，同时仍能承受后续高温步骤。

    2. **为什么当年选多晶硅而不是金属？**  
       现代 CMOS 多用**自对准栅**（self-aligned gate）：先做栅，再以栅为掩模注入源/漏，并做高温退火。铝等常用金属熔点低，扛不住这些高温；多晶硅熔点高、与硅工艺兼容。此外，N/P 管可用不同掺杂调节**功函数**（work function），便于设定阈值；栅、源漏用同一套硅基材料，步骤也更省。名称里的 “MOS” 来自早期**金属栅**结构；工业界改用多晶硅后，习惯上仍叫 MOS/MOSFET。

    3. **现代还用多晶硅吗？**  
       **不完全是。** 约 45 nm 起，逻辑主流程普遍改为 **高 κ 介质 + 金属栅**（high-k / metal gate, **HKMG**）：金属栅消除**多晶硅耗尽**（poly depletion），并更好匹配 high-k。更老的节点、部分模拟/高压/存储等工艺仍可见掺杂多晶硅（或硅化物）栅。讲义示意图画多晶硅，是经典结构，便于理解，不等于最先进逻辑节点仍只用多晶硅。
