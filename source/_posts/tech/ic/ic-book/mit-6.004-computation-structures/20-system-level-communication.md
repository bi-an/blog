---
title: MIT 6.004：L20 系统级通信
date: 2026-08-11 10:20:00
categories: ic
tags:
  - ic
  - digital-circuit
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L20 注解幻灯片。
>
> 源网页：[20.1 Annotated Slides | System-level Communication](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c20/c20s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L20：系统级通信（System-level Communication）

本讲从“接口比实现更长久”出发，回顾背板 **bus** 的电气与时序困境（传输线、反射、clock skew），总结网络经验：**单驱动、点对点、差分、时钟恢复（8b/10b）**；再看 PCIe/QPI 等串行链路，并以渐近复杂度比较 ring / mesh / hypercube 等拓扑。

## 1. 计算机系统中的技术（Computer System Technologies）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/45e577ee34a0057221f26fd2927a1b0c_Slide02.png" alt="Computer System Technologies" width="80%"/>

系统拼合多种技术；各部件有功能与接口规格。设计者按规格集成，不必深究内部实现。技术换代（更小更快更省电）时，**接口不变即可几乎无痛替换**——架构中真正长久的是接口。

## 2. 接口天长地久（Interfaces Last Forever）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f4550335571e98dc1b6c05495017e9e0_Slide03.png" alt="Interfaces Last Forever" width="80%"/>

长寿接口靠有用抽象：可靠字节流网络、窗口图形、日志文件系统等，屏蔽包/错误恢复/存储阵列细节。晶体管翻倍、网络 1→10 GHz、内存×4 时，不能每次从零重写。

反例：

- **Endianness**：IBM big-endian vs Intel little-endian——本地方便，联网传数值却终身麻烦（“一时方便，终身后悔”）
- 早期 IBM PC 扩展总线直接暴露当时 x86 引脚与协议——与特定 CPU 绑定，后续升级痛苦

## 3. 系统接口与模块化（System Interfaces & Modularity）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/73bceda39323f678815af8ad225d3655_Slide04.png" alt="System Interfaces and Modularity" width="80%"/>

演进：机柜间 ad-hoc 线缆 → 背板插板模块化（厂商互不兼容）→ 标准化背板促进竞争 → 性能再涨，背板带宽不够 → 又回到专用通道林立。工程现实最终推向 **通用单向点对点** 通道；异步点对点大体取代早期同步多信号总线。系统级通信多为线传，速率从 kHz 到 GHz 带来新电气问题。

## 4. 总线、互连，然后呢？（Buses, Interconnect, So…?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e486b6055e1054fd63f632b56d1bbfbc_Slide05.png" alt="Buses Interconnect" width="80%"/>

电路理论里导线是**等势节点**：电压处处相同、变化瞬时传到各端，距离被抽象掉。当电压变化速率相对电磁波渡越时间不再“慢”时，该模型失效。Heaviside 的电报方程早已说明信号沿导线有限速传播——高速下导线是 **transmission line**，长度与传播必须计入。

## 5. 真实导线的电气模型（Electrical Model for Real Wires）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ae1a13d3a3a915246744e406718b9346_Slide06.png" alt="Electrical Model for Real Wires" width="80%"/>

无穷小段模型参数：$R$（电阻）、$L$（自感）、$C$（对参考的电容）、$G$（绝缘泄漏）。高速、芯片/板级距离下近似**无损**传输线，特征阻抗 $Z_0$，波速 $\sim 1/\sqrt{LC}$。PCB 约 $Z_0\sim 50\,\Omega$，传播约 18 cm/ns。

电压阶跃沿导线传播；末端若不吸收能量会**反射回波**。需用匹配 $Z_0$ 的电阻端接；双向传播则两端都要端接。

## 6. 现实后果（Real-world Consequences）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5d9b7f957c5df0b7996cd71b1453c903_Slide07.png" alt="Real-world Consequences" width="80%"/>

残留能量会污染后续传输。通用药方是**给更多时间稳定**——高性能系统不可接受，故须减小储能效应：端接不准→反射；阻抗不连续→处处小回波；容性负载限制翻转速率→runt pulse；LC **ringing** 需等阻尼到合法电平。精心设计布线与驱动可把性能损失压到最低。

## 7. 空间与时间约束（Space & Time Constraints）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8f328b683a0f6538ea7a4d4f7188d776_Slide08.png" alt="Space and Time Constraints" width="80%"/>

信息沿时间保持 → **storage**；送到另一部件 → **communication**。通信耗时，时序预算必须计入：传播速度上限、部件间距、翻转过快引发的效应。时序模型要显式包含 **wire delay**。

## 8. 门、线与延迟（Gates, Wires, & Delays）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f91fd6bf986fb060d2a17fd387c14f31_Slide09.png" alt="Gates Wires and Delays" width="80%"/>

早期模型给门固定 $t_{\mathrm{PD}}$；实际输出延迟**依赖负载**。Jade 会按负载算有效传播延迟。减负载或加 **buffer** 驱动重载可加速。优化时常追踪重载慢线。以下转向系统级互连设计。

## 9. 接口标准：背板总线（Interface Standard: Backplane Bus）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3b5d7d1509610565c9da79485c47311b_Slide10.png" alt="Backplane Bus" width="80%"/>

可扩展性经典做法：主板插槽连接附加卡。信号含电源、时钟，以及：

- **地址线**：选端点（内存、控制寄存器等）
- **数据线**：传数据（早期常多比特并行）
- **控制线**：事务起止与应答

多槽可并联同一组线，靠地址区分目标。合称 **system bus**——按既定协议传数据的一组线。

## 10. 并行总线事务（A Parallel Bus Transaction）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/547404e26fe0a3f8c0a45210f6b73048_Slide11.png" alt="A Parallel Bus Transaction" width="80%"/>

CLK 的 assertion 边沿放信号、sample 边沿采样；周期须够传播并稳定。发起者 **bus master**“拥有”总线（可转移所有权）；指明操作、地址、写数据。**Slave** 在 sample 边沿认地址后执行，完成时用控制线应答，可读回数据。无响应时总线逻辑可超时报错。事务率不太高（如 <50 MHz）时此架构很实用。

## 11. 总线线即传输线（Bus Lines as Transmission Lines）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/cd84c313e3e6481efb00c866f5061eb2_Slide12.png" alt="Bus Lines as Transmission Lines" width="80%"/>

提速后问题放大：周期太短，主设备驱动→长总线传播→各接收端建立时间不够；**clock skew** 导致一卡新周期开驱动、另一卡仍在驱动 → 冲突噪声；连接器阻抗不连续产生多路小反射——像在大峡谷里喊话，回声淹没内容。总线最终留给低速，高速另寻他法。

## 12. 与此同时，箱外……（Meanwhile, Outside the Box…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/485c413a18de64d9b511a168f74f5008_Slide13.png" alt="Meanwhile Outside the Box" width="80%"/>

网络连接米级距离：比特组成带目的地址与校验的 **packet**，可请求重传。协议栈分层：物理层收发包并检错；网络层寻址路由；传输层提供可靠字节流与 **flow control**。关键思想：在 “best effort” 包网上堆可靠通道——上层可恢复下层错误，比每层 100% 可靠更便宜稳健。

## 13. 经验：单驱动、点对点（Lessons learned: Single driver; point-to-point）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/30dd6dee964db2fc6a3ff3fe61c70d3f_Slide14.png" alt="Single driver point-to-point" width="80%"/>

共享线上多驱动/多接收电气问题多；减速能缓解但高性能不许。网络经验：**point-to-point**（单驱动↔单接收）最快最干净。**差分信号**测两线电压差，共模噪声抵消——几乎所有高速链路都用。

## 14. 经验：时钟恢复（Lessons learned: Clock recovery）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/aae7afc6203cf297aa4711386ff93fd4_Slide15.png" alt="Clock recovery" width="80%"/>

不必另送时钟：接收端用跳变推断部分边沿，再以标称周期 + **PLL** 生成本地时钟。包前加 **training sequence**；特殊分隔序列标数据起点。为保足够跳变，常用 **8b/10b**（8 消息比特→10 传输比特，保证至多每 6 bit 时间有跳变）。真正只需一路比特流即可同时恢复时钟与数据。

## 15. 串行点对点通信（Serial, Point-to-point Communications）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/fa2493bc98a3f94ccd5bf2c4a85356c4_Slide16.png" alt="Serial Point-to-point" width="80%"/>

局域网从共享介质变为点对点（如 BaseT 收发各一对差分线），经交换机/路由器多跳转发。系统内互连同理：点对点 + 交换路由。各链路独立，多链路可并行提供大带宽；交换机少量缓冲处理短暂争用。

## 16. 改进总线（Improving on the Bus）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/db70c975f5486a29c04f761fcad5089a_Slide17.png" alt="Improving on the Bus" width="80%"/>

串行点对点取代并行总线：无共享、无 clock skew、电气环境可控 → GHz 级。要更高吞吐可**多 lane 并行**，用逻辑重组分包。扩展卡仍插主板，但接到的是点对点链路而非并行总线。

## 17. 当今计算机中的通信（Communications in Today’s Computers）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/30c09568fb25eed78047bf8e4799fd03_Slide18.png" alt="Communications in Today Computers" width="80%"/>

例：Intel Core i7 系——CPU 直连内存求带宽；其余经 **QPI**（每向 20 路差分，可达每向每秒 64 亿次 20-bit 传输）。USB、PCIe、网口、SATA、音频等亦为串行链路。有了足够强的通用互连，何必堆一堆专用通道——有如魔戒“一戒御众”。

## 18. 串行链路例：PCI Express（Example serial link: PCI Express (PCIe)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4af1b80e2a5679eb8fd71a7d3a59c9c2_Slide19.png" alt="PCI Express" width="80%"/>

PCIe Gen2 单 **lane**：5 Gb/s，LVDS，约 100 Ω 特征阻抗。物理层：训练序列 + 起止定界 + 载荷；链路层用序号与 CRC 检丢包并重传、做流控；事务层重组多 lane、按头识别接收方。8 lane 可达约 4 GB/s，够显卡等高性能外设。主板通信已从并行总线转向少量串行点对点——更快、更可靠、更省电、更紧凑。

## 19. 通信拓扑（Communication Topologies）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/054e50bf15e14e06a02ffcd0b08f6a72_Slide20.png" alt="Communication Topologies" width="80%"/>

连接 $N$ 个需互发消息的部件（如多核）。约定：每点对点链路代价 1、传一跳时间 1、链路可并行。渐近看吞吐、最坏延迟、硬件代价。

- **总线**：吞吐 $O(1)$，延迟 $O(1)$，代价 $O(n)$
- **Ring**：吞吐/代价 $O(n)$，最坏延迟 $O(n)$——适合流水线或延迟不敏感场景

## 20. 二次代价拓扑（Quadratic-cost Topologies）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/abf1f2bce6f5671439c836a6509b8d19_Slide21.png" alt="Quadratic-cost Topologies" width="80%"/>

- **完全图**：链路 $O(N^2)$ → 吞吐与代价 $O(N^2)$，延迟 1
- **Crossbar**：每时隙每行/列一条消息，吞吐 $O(n)$、延迟 1，开关数 $N^2$ → 代价仍 $O(N^2)$

## 21. 网格拓扑（Mesh Topologies）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7cc9ecd1103469f2cf4a1f71e8f1c553_Slide22.png" alt="Mesh Topologies" width="80%"/>

2D/3D mesh：每节点连固定邻居 → 链路数 $\propto N$，吞吐与代价 $O(n)$。最坏延迟：2D 为 $O(\sqrt{n})$，3D 为 $O(\sqrt[3]{n})$。布局规整、每节点硬件常量、延迟适中 → 实验多核常用 2D 四邻接 mesh。

## 22. 对数延迟网络（Logarithmic-latency Networks）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/259c477b7c50dddd21cbfcfbc2b79b50_Slide23.png" alt="Logarithmic-latency Networks" width="80%"/>

**Hypercube**、**tree** 提供对数级延迟。CM-1 Connection Machine 用超立方连接多达 65536 个简单处理器（各连 16 邻）；后期改树，且靠近根的链路容量更大。

## 23. 通信技术：延迟（Communication Technologies: Latency）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/03892845f4ee4af09a7019f31d899a56_Slide24.png" alt="Communication Technologies Latency" width="80%"/>

三维世界中部件最坏距离下界 $O(\sqrt[3]{N})$，平面布局 $O(\sqrt{N})$——延迟应反映物理距离。总线/crossbar 上 $N$ 个连接的容性负载也抬高下界。**Mesh** 随 $N$ 增长不必强行加长线，对连接上千处理器的高容量片上网络很有吸引力。

## 24. 通信的未来（Communications Futures）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/00c2729745ea1783c3b84e55f56e8663_Slide25.png" alt="Communications Futures" width="80%"/>

总结：点对点已成系统级主流；超高带宽内存通道仍用多信号并行但工程极谨慎。无线连接移动设备，并研究自动发现附近外设。多核将有数十到数百核，片上网络拓扑（高带宽 + 低延迟）仍是活跃研究——未来十年对片上网络工程师很有戏。
