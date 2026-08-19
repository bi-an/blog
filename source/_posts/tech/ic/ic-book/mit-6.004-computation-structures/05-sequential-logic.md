---
title: MIT 6.004：L05 时序逻辑
date: 2026-08-11 10:05:00
categories: ic
tags:
  - ic
  - digital-circuit
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L05 注解幻灯片。
>
> 源网页：[5.1 Annotated Slides | Sequential Logic](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c5/c5s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L05：时序逻辑（Sequential Logic）

本讲在组合逻辑之上引入**状态**（state）与**存储**（memory）：从电容存储与反馈双稳态，到 D 锁存器、边沿触发 D 寄存器，再到单时钟同步电路的建立/保持时间与 $t_{\mathrm{CLK}}$ 约束。

## 1. 还不能造的东西（Something We Can’t Build (Yet)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3fd216fac0d8a4dca530c15590a9af85_Slide02.png" alt="Something We Can't Build Yet" width="70%"/>

上一讲学会了按功能规格做组合逻辑：输出只由**当前**输入决定。

但有一类简单器件做不到：灯作输出、按钮作输入——灯灭时按一下灯亮，灯亮时再按一下灯灭。输出并不只是当前输入的函数：第奇数次按下亮、偶数次灭，器件在“记住”上次是奇次还是偶次。这种依赖输入历史的器件称为有**状态**。

更细微的一点：关心的是按钮从松开到按下的**跳变时刻**，而不只是“当前是否按下”。内部状态使相同输入可产生不同输出；纯组合器件做不到。本讲要把状态写进电路。

## 2. 数字状态：想造什么（Digital State: What We’d Like to Build）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/56754b6fe38357d184b4d35ec1948f0c_Slide03.png" alt="Digital State What We'd Like to Build" width="70%"/>

引入**存储器**抽象：用一个或多个比特编码系统当前状态，这些比特作为数字值出现在输出（图中 “Current State”）。

当前状态与当前输入一起进入一块组合逻辑，产生两组输出：

- **下一状态**（next state）：比特数与当前状态相同
- **系统输出**：对外可见的信号

组合逻辑的功能规格（真值表或布尔方程）规定：下一状态与系统输出如何由当前状态与当前输入决定。

存储器有两个输入：`LOAD` 控制何时用下一状态替换当前状态，以及给出下一状态数据的输入。周期性触发 `LOAD`，就得到当前状态序列；序列中每一态由上一态与触发时刻的输入决定。

含组合逻辑与存储器的电路称为**时序逻辑**（sequential logic）。若存储器存 $K$ 比特，可能状态数至多为 $2^K$。本章讨论如何造可加载的存储元件；下一章讨论如何系统化设计时序行为。

## 3. 用电容做存储（Memory: Using Capacitors）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/12a0e958520be77acd5c57e88e385643_Slide04.png" alt="Memory Using Capacitors" width="70%"/>

比特已用电压表示，可考虑用电容存某电压。电容是无源二端器件：两平行板隔绝缘体，电荷 $Q$ 与电压差 $V$ 满足 $Q=CV$。接到较高电压叫**充电**，接到较低电压叫**放电**。

电容存储单元示意：电容一端接稳定参考；另一端经 NFET 开关接到 **bit line**；开关栅极接 **word line**。

- **写**：把 bit line 驱动到目标电压（数字 0/1），再令 word line 为 HIGH，电容充/放到与 bit line 同压；再令 word line 为 LOW，关断开关，理想情况下电荷可一直留在内部极板。
- **读**：先把 bit line 充到中间电压，再开 word line，使 bit line 与存储电容**电荷共享**。存 1（较高压）时电荷流向 bit line，电压略升；存 0 则略降。变化通常很小，需**灵敏放大器**（sense amp）检出并还原成合法数字电压。

- **优点**：存储电容极小，现代 IC 可在廉价 **DRAM**（dynamic random-access memory）上集成数十亿比特，单位比特成本低。
- **缺点**：读写需要复杂的操作，所以访问慢；必须注意在外部电噪声干扰下，小心地保持储能电容器上的电荷；NFET 关断仍有泄漏，当前技术下须约每 10 ms **刷新**（refresh）——读出再写回。所以我们需要搭配精心设计的模拟电子电路（analog electronics） [^along-with-designed-electronics]。

[^along-with-designed-electronics]: 译者注：模拟电子电路（analog electronics）主要指以下几个关键的模拟电路组件和机制：

    - 敏感放大器（Sense Amplifier）：
      - 在读取电容存储的数据时，电容释放或吸收的电荷极少，只能导致位线（Bit line）产生非常微小的电压变化（远未达到标准数字电路的 0 或 1 电平）。
      - 需要专门设计的高灵敏度模拟放大器来检测这一微弱的连续电压差，并将其放大还原为标准的数字逻辑电平（逻辑 0 或逻辑 1）。

    - 微小电荷与连续电压的处理（Charge Sharing & Leakage Control）：
      - 电容上的电荷存储和衰减（泄漏）、位线上的电荷共享（Charge sharing）都是连续变化的模拟物理过程，而非瞬间完成的离散数字状态。
      - 电路需要精准控制 NFET 晶体管开关的导通电阻、阈值电压以及微小的泄漏电流（Leakage current），这些器件特性的设计和优化都属于模拟电路设计的范畴。

或许我们可以通过设计一种利用“反馈”持续刷新存储信息的电路来克服电容式存储的缺点。

## 4. 用反馈做存储（Memory: Using Feedback）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c9962578f708737165ef496378f1337d_Slide05.png" alt="Memory Using Feedback" width="70%"/>

两只组合反相器接成**正反馈**环：一输入为数字 0 → 其输出为 1 → 另一反相器输出为 0，又接回原输入。只要接电源与地，该配置在噪声下仍可保持；两线数字值对调也是稳定的。有两种稳定配置，称为**双稳态**（bi-stable）存储元件。

把两反相器看成一个系统，其 VTC 给出 $V_{\mathrm{OUT}}$ 与 $V_{\mathrm{IN}}$ 关系；输出接回输入又要求 $V_{\mathrm{IN}}=V_{\mathrm{OUT}}$。图解求交点：两端交点为**稳定**——$V_{\mathrm{IN}}$ 小扰动几乎不影响 $V_{\mathrm{OUT}}$，系统会回到稳态。中间交点为**亚稳态**（metastable）：理论上可无限停在该电压，但极小扰动就会迅速滑向某一稳态。作存储用时须避免进入亚稳态（下章再谈）。

## 5. 可置位存储元件（Settable Memory Element）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/90b3c2fd995f56f0b76ec1ddfcf5dc32_Slide06.png" alt="Settable Memory Element" width="70%"/>

用 **2 选 1 MUX** 做可置位存储：MUX 输出作状态输出 $Q$，并回接到 MUX 的 $D_0$；$D_1$ 作数据输入；选择端作加载信号（此处称 **gate**，$G$）。

- $G=\mathrm{LOW}$：输出经 $D_0$ 回环，形成上节的双稳态正反馈；电路出现环路，不再是组合电路。
- $G=\mathrm{HIGH}$：输出跟随 $D_1$（数据输入）。

加载：将 $G$ 置 HIGH 足够久，使 $Q$ 有效且稳定；真值表上 $G=1$ 时 $Q$ 跟随 $D$，变化时序由 MUX 的 $t_{\mathrm{PD}}$ 决定。再把 $G$ 置 LOW，进入“记忆模式”，正反馈无限保持稳定 $Q$。

## 6. 新器件：D 锁存器（New Device: D Latch）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dda8c360d0910e5eda03ff8bfa9d567d_Slide07.png" alt="New Device D Latch" width="70%"/>

该存储器称为 **D 锁存器**（D latch），原理图符号如图。

- 门控 $G$ 为 HIGH：**打开**（open），信息从 $D$ 流向 $Q$
- $G$ 为 LOW：**关闭**（closed），进入记忆模式，记住 $G$ 由 HIGH→LOW 时 $D$ 上的值

右侧时序图：稳定段为恒定 LOW/HIGH，变化段画成多次跳变。$G$ 为 HIGH 时，$D$ 稳定后至多再经 $t_{\mathrm{PD}}$，$Q$ 到达新稳定值。

理论期望：$G$ 变 LOW 后，$Q$ 保持跳变瞬间 $D$ 上的值。但一般组合器件在输入跳变后，$t_{\mathrm{CD}}$ 到 $t_{\mathrm{PD}}$ 之间输出可任意变化——若 $G$ 的 1→0 使 $Q$ 短暂失效，要记的正是 $Q$ 上的值！因此必须保证 $G$ 的 1→0 **不影响** $Q$。[^dlatch-impl]

[^dlatch-impl]: 译者注：上文门控信号 $G$（即后文的 CLK）在 CMOS 中接到**钟控反相器**或**传输门**的栅极。以下给出两种基本单元与完整 D 锁存器电路，展示 CLK 接到 MOS 管的哪个端点。电路图由 [Schemdraw](https://schemdraw.readthedocs.io/en/stable/index.html) 绘制（也可参考我的另一篇文章「Schemdraw 绘制 IC 原理图」）。

    **钟控反相器（Clocked Inverter）**   标准 CMOS 反相器各串一个时钟管，CLK/CLK̄ 接到时钟管的栅极。CLK=HIGH 时反相器正常工作；CLK=LOW 时输出高阻（floating）。有两种常见串联顺序：

    | ![钟控反相器实现一](/assets/images/tech/ic/ic-book/mit-6.004-computation-structures/05-seq-clk-inv-01.svg) | ![钟控反相器实现二](/assets/images/tech/ic/ic-book/mit-6.004-computation-structures/05-seq-clk-inv-02.svg) |
    |:-:|:-:|
    | 钟控反相器实现一：时钟管在外侧 | 钟控反相器实现二：时钟管在中间 |

    <details>
    <summary>Schemdraw 绘图代码：钟控反相器实现一</summary>

    {% include_code lang:python tech/ic/05-seq-clk-inv-01.py %}

    </details>

    <details>
    <summary>Schemdraw 绘图代码：钟控反相器实现二</summary>

    {% include_code lang:python tech/ic/05-seq-clk-inv-02.py %}

    </details>

    **传输门（Transmission Gate）**   NFET（栅 = CLK）与 PFET（栅 = $\overline{\text{CLK}}$）并联。CLK=HIGH 时双向导通（无阈值损失）；CLK=LOW 时断开。

    ![传输门](/assets/images/tech/ic/ic-book/mit-6.004-computation-structures/05-seq-tg-01.svg)

    <details>
    <summary>Schemdraw 绘图代码：传输门</summary>

    {% include_code lang:python tech/ic/05-seq-tg-01.py %}

    </details>

    **D 锁存器**（双钟控反相器）  CI1（CLK=HIGH 导通）将 D 反相至 $\overline{Q}$；CI2（CLK=LOW 导通）将 $\overline{Q}$ 反相反馈回 D 节点，构成保持环。

    ![D 锁存器](/assets/images/tech/ic/ic-book/mit-6.004-computation-structures/05-seq-dlatch-01.svg)

    <details>
    <summary>Schemdraw 绘图代码：D 锁存器</summary>

    {% include_code lang:python tech/ic/05-seq-dlatch-01.py %}

    </details>

## 7. 请求宽容（A Plea for Lenience）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9e6c455c2e0b75885da4d747316ec456_Slide08.png" alt="A Plea for Lenience" width="70%"/>

因此，存储元件选用的是**宽容的**（lenient）MUX；宽容 MUX 的真值表见上图。在下列任一条件下，即使输入发生跳变，输出仍保持有效且稳定：

1. 将 $G$ 置 HIGH 以加载锁存器时：一旦 $D$ 已有效稳定满 $t_{\mathrm{PD}}$，即可保证 $Q$ 稳定有效，且取值与 $D$ 相同，与 $Q$ 的初值无关。
2. 若 $Q$ 与 $D$ 均已有效稳定满 $t_{\mathrm{PD}}$，则此后 $G$ 上的跳变不影响 $Q$——正是靠这一点，$G$ 才能做 1→0 而不污染 $Q$。
3. 若 $G$ 为 LOW，且 $Q$ 已稳定至少 $t_{\mathrm{PD}}$，则此后 $D$ 上的跳变不影响输出。

宽容性是否足以保证锁存器正常工作？还不够——除非我们小心保证信号在正确时刻稳定，才能用上 MUX 的宽容行为。

## 8. 还须一点纪律（… With a Little Discipline）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/595e10687a103e0484e86d17d0a9238f_Slide09.png" alt="With a Little Discipline" width="70%"/>

保证锁存器按预期工作，需按下列步骤操作：

1. 先在 $G$ 为 HIGH 时，把待存值放到 $D$；经 $t_{\mathrm{PD}}$ 后，即可保证该值在 $Q$ 上稳定有效。这就是上节条件 1。
2. 再等一个 $t_{\mathrm{PD}}$，让 $Q'$ 输入上新值的信息传遍锁存器内部电路。此时 $D$ **与** $Q'$ 都已稳定至少 $t_{\mathrm{PD}}$，从而满足上节条件 2。
因此若 $D$ 稳定满 $2\,t_{\mathrm{PD}}$，$G$ 上的跳变就不会影响 $Q$。对 $D$ 的这一要求称为锁存器的**建立时间**（setup time）：即 $G$ 做 HIGH→LOW 之前，$D$ 必须已稳定有效多久。

3. 这时可将 $G$ 置 LOW，同时仍保持 $D$ 稳定有效。再经一个 $t_{\mathrm{PD}}$，让新的 $G$ 值传遍内部电路后，就满足了上节条件 3，此后 $D$ 上的跳变不再影响 $Q$。
对 $D$ 稳定性的这一进一步要求称为锁存器的**保持时间**（hold time）：即 $G$ 跳变之后，$D$ 还必须继续稳定有效多久。

| 时序参数 | 英文名称 | 概念定义 | 针对的对象（作用接口与角色） |
|----------|----------|----------|------------------------------|
| 建立时间 ($t_{\mathrm{SETUP}}$) | Setup Time | 时钟沿到达之前，数据必须保持稳定不变的最短时间。 | 输入端 $D$（外部输入信号需满足的约束/条件） |
| 保持时间 ($t_{\mathrm{HOLD}}$) | Hold Time | 时钟沿到达之后，数据仍需继续保持稳定不变的最短时间。 | 输入端 $D$（外部输入信号需满足的约束/条件） |
| 传播延迟 ($t_{\mathrm{PD}}$) | Propagation Delay | 从时钟触发沿到达起，到输出彻底稳定到最终正确逻辑状态所需的最长时间。 | 输出端 $Q$（寄存器自身产生的物理延迟/表现） |
| 污染延迟 ($t_{\mathrm{CD}}$) | Contamination Delay | 从时钟触发沿到达起，到输出开始发生改变/受到影响所需的最短时间。 | 输出端 $Q$（寄存器自身产生的物理延迟/表现） |

建立时间与保持时间合称**动态纪律**（dynamic discipline），锁存器要正确工作就必须遵守。

简言之，动态纪律要求：在 $G$ 发生跳变的**前后**，$D$ 都必须稳定有效。只要电路设计遵守动态纪律，就能保证门控做 HIGH→LOW 时，该存储元件可靠地存下 $D$ 上的信息。

## 9. 试一把（Let’s Try it Out!）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1bb1766344daa10aacf0a6fe11085d7a_Slide10.png" alt="Let's Try it Out" width="70%"/>

把锁存器用作时序系统的存储：打开锁存器（$G$ HIGH），让新状态传到 $Q$（当前状态），再经组合逻辑更新下一状态。问题：$G$ 若 HIGH 过久，系统形成环路，新状态沿环快速变化，加载计划失败。

因此 $G$ 为 HIGH 的窗口既要够长以满足动态纪律，又要够短，使锁存器在新状态绕环一圈前重新关闭。精确间隔几乎无法保证——只有时序上下界，没有精确间隔。需要的是标记**某一瞬间**的加载信号，而不是一段时间。

## 10. 不可靠的控制系统（Flakey Control Systems）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/86301e39b62cfd877a638af7d3f1cbaa_Slide11.png" alt="Flakey Control Systems" width="70%"/>

类比：收费站闸门前排队的车 = 时序逻辑中的状态序列；闸门 = 锁存器。闸门关闭时车等候；打开后第一辆驶出——但何时关闸很难：开得够久让第一辆过，又不能太久让后面的车也过。与单锁存器作存储时的问题相同。

## 11. 解法：擒纵策略（双闸）（Solution: Escapement Strategy (2 Gates)）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/efb478a7f0db6cea63bfe00f48235bed_Slide12.png" alt="Escapement Strategy Two Gates" width="70%"/>

用**两道闸**：起初 Gate 1 开、Gate 2 关，恰允许一辆进入；某一瞬间关 Gate 1、开 Gate 2，让站内那辆驶出，并挡住后续车辆。重复两步，逐辆处理。关键：**任意时刻不存在同时穿过两道闸的通路**。

这与机械钟表的**擒纵机构**（escapement）相同：保证发条齿轮一次只进一齿，防止整天一下子转完。观察输出：Gate 2 每次打开后不久驶出一辆；车辆按 Gate 2 打开间隔通过。把该思路用于时序逻辑的存储元件。

## 12. 边沿触发 D 寄存器（Edge-triggered D Register）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e8b8c50b21bdf0bcaeedc4539a219633_Slide13.png" alt="Edge-triggered D Register" width="70%"/>

用两个背靠背锁存器做 **D 寄存器**（D register）：加载信号通常叫**时钟**（clock）；$D$/$Q$ 角色与锁存器相同。

从外部看，$D$ 接**主锁存器**（master），$Q$ 接**从锁存器**（slave）。时钟在进主锁存器前反相：主开则从关，反之亦然——任意时刻没有从寄存器 $D$ 到 $Q$ 的通路。

时钟反相器延迟可能会引发担心：上升沿时是否会有短暂两 Gate 都 HIGH？我们可以换用“$G$ LOW 时打开、$G$ HIGH 时关闭”类型的主锁存器，往往不必另加反相器。

因锁存器内正反馈双稳态（具有两个可以长期保持的稳定工作状态），寄存器也常被称为 **flip-flop** （触发器）。

## 13. D 寄存器波形（D-Register Waveforms）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/9e3ee853fa307630648767ab19e47b17_Slide14.png" alt="D-Register Waveforms" width="70%"/>

整体行为简单：时钟 **0→1 上升沿**采样 $D$，并保持到下一上升沿；$Q$ 即寄存器中存的值。

时钟 LOW→HIGH 为**上升沿**，HIGH→LOW 为**下降沿**。主锁存器输出记为 STAR：

- **上升沿**：主由开→关，采样输入并进入记忆；时钟为 HIGH 期间 STAR 保持稳定。
- **下降沿**：主打开，STAR 再跟随 $D$（延迟为锁存器 $t_{\mathrm{PD}}$）。

从锁存器：上升沿时打开，输出跟随 STAR；但时钟 HIGH 时主已关，STAR 稳定，故 $Q$ 在可能的一次跳变后也稳定。下降沿时从由开→关，采样 STAR；时钟 LOW 期间 $Q$ 保持稳定。

单独看 $Q$：只在上升沿从锁存器打开时变化，故称**正边沿触发 D 寄存器**（positive-edge-triggered D register）。原理图时钟端用小三角标记。

## 14. 关于那个保持时间…（Um, About That Hold Time…）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e47ea050dbc032c469af69b71029ec8b_Slide15.png" alt="About That Hold Time" width="70%"/>

下降沿时从锁存器由开→关，其输入 STAR 必须满足从锁存器的建立/保持时间。麻烦在于：主同时打开，STAR 可能在时钟边沿后很快变化。主的**污染延迟** $t_{\mathrm{CD}}$ 告诉我们其输出旧值在下降沿后还能稳多久；从的**保持时间**告诉我们边沿后其输入还须稳多久。

正确工作要求：主的 $t_{\mathrm{CD}}$ $\ge$ 从的保持时间。分析须考虑工艺偏差、温度、电源等。必要时可在主从之间加额外门延迟（如两级反相器）增大相对下降沿的污染延迟。从锁存器保持时间问题只能靠改电路设计解决。

## 15. D 寄存器时序（D-Register Timing）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/cbad449926e607f01e053ef60ff0aad3_Slide16.png" alt="D-Register Timing" width="70%"/>

D 寄存器时序规格小结：

- $t_{\mathrm{PD}}$：上升沿后，$Q$ 变为有效稳定的上界
- $t_{\mathrm{CD}}$：上升沿后，旧 $Q$ 仍保持有效的下界

二者均相对时钟上升沿度量。寄存器设计为宽容：若新旧 $Q$ 相同，上升沿期间 $Q$ 稳定性有保证——$t_{\mathrm{CD}}$/$t_{\mathrm{PD}}$ 仅在 $Q$ 实际变化时适用。

为保证主锁存器正确，$D$ 须满足主的建立/保持：

- $t_{\mathrm{SETUP}}$：上升沿**前** $D$ 须有效稳定多久
- $t_{\mathrm{HOLD}}$：上升沿**后** $D$ 须再保持多久

使用厂商库中的 D 寄存器时，要从数据手册查这四个参数，再分析整电路时序。

## 16. 单时钟同步电路（Single-clock Synchronous Circuits）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/3c5ff1860eb2c9a36a5c40241c82a154_Slide17.png" alt="Single-clock Synchronous Circuits" width="70%"/>

6.004 中使用寄存器的约定称为**单时钟同步纪律**（single-clock synchronous discipline）：

上图左边的电路中，带边沿触发的符号的方形表示寄存器，小云朵表示组合逻辑电路。

- 寄存器输入到输出无组合通路，整个电路无组合环。换句话说，从系统输入和寄存器输出到寄存器输入的路径不会两次经过同一组合块。
- 所有时钟器件共享**同一**周期时钟。使用多时钟域也可以，但跨域时序很难分析，同钟更简单。
- 各数据信号何时变化大体不重要；关键是接到寄存器输入的信号稳定够久以满足建立时间，并保持够久以满足保持时间。

选时钟周期大于“寄存器输出 → 寄存器输入”每条路径的 $t_{\mathrm{PD}}$ 再加上建立时间，即可保证遵守动态纪律：
$$t_{\mathrm{CLK}} > t_{\mathrm{PD,path}} + t_{\mathrm{SETUP}}$$
额外好处：按这种方式选时钟周期后，上升沿瞬间电路中没有其它会引入噪声的逻辑跳变，因而更新各寄存器存储状态时不应有噪声问题。[^clk-noise-decouple]

[^clk-noise-decouple]: 译者注：这句话点出单时钟同步电路（single-clock synchronous system）的一个精妙优势——通过合理控制时钟周期，把电路噪声与状态更新彻底“解耦”（错开）。可从三层理解：

    1. **逻辑翻转是电路噪声的主要来源。** CMOS 中信号 0↔1（逻辑翻转/过渡）会产生瞬时大电流（充放电电流及导通短路电流）。电流突变导致电源电压波动（电源噪声 / ground bounce），也会经寄生电容干扰邻近信号线（串扰 crosstalk）。

    2. **“按这种方式选择时钟周期”做了什么。** 指将 $T_{\mathrm{CLK}}$ 设得足够长：
       $$T_{\mathrm{CLK}} \ge t_{\mathrm{PD,reg}} + t_{\mathrm{PD,comb}} + t_{\mathrm{setup}}$$
       从而保证：上一上升沿触发后，组合逻辑里所有“中间过渡与信号震荡/翻转”（noise-inducing logic transitions）都有足够时间完全停止并稳定下来。

    3. **为何上升沿到来时没有噪声问题。** 下一时钟上升沿触发寄存器锁存/更新新状态的那一瞬，组合逻辑已“静止”很久，没有线缆在发生信号翻转。电路内部没有其它翻转带来的电源噪声或串扰，寄存器可在安静环境下精准采样并写入新状态，稳定性与可靠性更高。

    形象比喻：组合逻辑计算像拍集体照前大家调整姿势、换位置（杂乱动作与噪音）；足够长的时钟周期是给所有人留足时间站好不动。上升沿（快门按下）到来时所有人都已静止，拍出的照片（采样写入的数据）不会有模糊或重影（噪声干扰）。

## 17. 单时钟系统中的时序（Timing in a Single-clock System）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7fe759d2481ff7e0b2ec8116ac4e3345_Slide18.png" alt="Timing in a Single-clock System" width="70%"/>

分析某条路径：上游寄存器 → 组合逻辑 → 下游寄存器（逻辑输出记 STAR）。大系统有多条路径，须逐条分析以找出决定最小可用时钟周期的路径（CAD 工具常代劳）。

上升沿触发上游寄存器：$Q_{\mathrm{R1}}$ 在至少 $t_{\mathrm{CD,REG1}}$ 内保持旧值，并在 $t_{\mathrm{PD,REG1}}$ 前达到新稳定值，直至下一上升沿。组合逻辑的 $t_{\mathrm{CD}}$/$t_{\mathrm{PD}}$ 再决定 STAR 最早失效与最晚稳定的时刻。

对下游 REG2：

- $t_1$ = REG1 的 $t_{\mathrm{CD}}$ + 组合逻辑的 $t_{\mathrm{CD}}$：上升沿后 STAR 仍有效多久；须 $t_1 \ge$ REG2 的保持时间。
- $t_2$ = REG1 的 $t_{\mathrm{PD}}$ + 组合逻辑的 $t_{\mathrm{PD}}$ + REG2 的建立时间：下一上升沿最早允许发生的时刻；须 $t_2 \le t_{\mathrm{CLK}}$（时钟周期）。过早则违反 REG2 动态纪律。

每条寄存器到寄存器路径都须满足这两个不等式。$t_{\mathrm{CLK}}$ 不等式中，上游 $t_{\mathrm{PD}}$ 与下游建立时间从可用于“有用逻辑工作”的时间里抠掉，故设计者倾向选用这两项更小的寄存器。

若上下游之间**没有**组合逻辑（移位寄存器、数字延迟线等），第一不等式要求上游 $t_{\mathrm{CD}} \ge$ 下游保持时间；实践中污染延迟常小于保持时间，往往要插入哑逻辑（如两级反相器）制造足够 $t_{\mathrm{CD}}$。

还须考虑**时钟偏斜**（clock skew）：因为时钟信号到达不同寄存器存在“时间差”（即时钟偏斜），为了确保数据不报错，下游寄存器不得不预留更宽裕的“稳定观察期”（即增加了等效的建立时间和保持时间）。

时钟周期 $t_{\mathrm{CLK}}$ 刻画了系统性能。Intel 等按不同时钟频率卖处理器——芯片往往相同，工艺波动使部分芯片 $t_{\mathrm{PD}}$ 更好，可支持更小 $t_{\mathrm{CLK}}$、更高频率；测速后挑快的卖更高性能档。

## 18. 模型：离散时间（Model: Discrete Time）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/d377c41298081890008a96113d85dfba_Slide19.png" alt="Model Discrete Time" width="70%"/>

用 D 寄存器作时序系统的存储很合适：每个时钟上升沿加载下一状态，在余下时钟周期内作为当前状态出现在输出。组合逻辑用当前状态与输入计算下一状态与输出。一串上升沿与输入产生状态序列，进而产生输出序列。下一章引入**有限状态机**（finite state machine）抽象，便于设计时序系统。

## 19. 时序电路时序分析（Sequential Circuit Timing）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5bbe99ae9bbcd7ce79d96bab7d97c5ff_Slide20.png" alt="Sequential Circuit Timing" width="70%"/>

对图示时序系统套用上述分析（寄存器与组合逻辑时序参数如图）：

- 组合逻辑 $t_{\mathrm{CD}}$ 未给：要正确工作，寄存器与逻辑的 $t_{\mathrm{CD}}$ 之和须 $\ge$ 寄存器保持时间 → 逻辑 $t_{\mathrm{CD}}$ 至少 **1 ns**。
- 最小 $t_{\mathrm{CLK}}$：须大于寄存器与逻辑的 $t_{\mathrm{PD}}$ 之和再加上建立时间 → 最小时钟周期 **10 ns**。
- 相对上升沿，**Input** 的建立时间 = 逻辑 $t_{\mathrm{PD}}$ + 寄存器建立时间 = **7 ns**；保持时间 = 寄存器保持时间 − 逻辑 $t_{\mathrm{CD}}$ = **1 ns**。这样 Next State 才能分别满足寄存器的建立/保持。

## 20. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/1458621dc9f23bbed582c1796a9b25cd_Slide21.png" alt="Summary" width="70%"/>

本讲完成时序逻辑入门：几乎所有数字系统都是时序系统，并遵守动态纪律施加的时序约束。下次看到 “1.7 GHz 处理器” 广告，就知道 “1.7” 从何而来——由可满足建立时间的最小 $t_{\mathrm{CLK}}$（以及工艺筛选）决定。
