---
title: Schemdraw 绘制 IC 原理图
date: 2026-08-13 11:37:33
categories: ic
tags:
  - ic
  - digital-circuit
---

## 简介

[Schemdraw](https://schemdraw.readthedocs.io/) 是一个用 Python 绘制电路原理图的库。使用时逐一添加元件并设置其朝向与标签，库按添加顺序维护画笔位置，自动完成连线衔接，最终导出 SVG 或 PNG。

与 KiCad、Fritzing 等图形化工具不同，Schemdraw 没有交互式编辑器，图形完全由代码生成。这一特点使它天然适合**需要版本控制与可复现性**的场景——文档、讲义、论文附图等，图形随代码一起纳入版本管理。

```bash
pip install schemdraw
```

放置元件前，可先对照官方图示了解各符号的外形与默认朝向：[Basic Elements（electrical）](https://schemdraw.readthedocs.io/en/stable/elements/electrical.html)。

## 核心对象：Drawing 与 Element

画图时主要打交道的是两个类：

| 类 | 含义 | 在程序中的角色 |
|----|------|----------------|
| **`schemdraw.Drawing`** | **一张**原理图画布 | 容器：持有已添加的元件、全局样式，以及当前画笔位置 `here` |
| **`schemdraw.elements.Element` 及其子类** | 单个电路符号 | `Resistor`、`PFet`、`Line`、`Ground` 等都是 `Element` 的子类；先构造实例，再交给 `Drawing` 放置 |

`Drawing` 支持上下文管理器（`with ... as d`）：进入时创建画布，退出时按配置显示或保存。常用写法：

```python
import schemdraw
import schemdraw.elements as elm

with schemdraw.Drawing() as d:          # d 是 Drawing 实例
    d.config(fontsize=12, color='black', bgcolor='white')
    r = d.add(elm.Resistor().right().label('1k'))   # 构造 Element，再加入画布
    d.add(elm.Capacitor().down().at(r.end).label('10uF'))
    d.save('out.svg')
```

要点：

1. **`elm.Resistor()` 等**只构造符号对象（可链式设置朝向、标签），**尚未上画布**。
2. **`d.add(...)`** 才真正放置：按入笔（Placement）/ 出笔（Drop）更新 `d.here`，并返回已放置的元件，便于用 `r.end`、`MP.gate` 等锚点继续布线。
3. 整张图的画笔状态（`d.here`、全局样式）在 `Drawing` 上；单个符号的几何与锚点在 `Element` 上。后续各节（坐标系、朝向、链式接口）均在说明这两者如何配合。

## 坐标系与画笔

`Drawing` 维护一个全局画笔坐标 **`d.here`**。每次 `d.add()` 放置元件时，有两件独立的事情发生：

| 概念 | 含义 | 控制方式 |
|------|------|----------|
| **Placement（入笔）** | 元件以哪个引脚对齐到起始坐标；起始坐标默认为当前 `d.here` | `.at(坐标或锚点)` 改起始坐标；`.anchor(名)` 改对齐引脚 |
| **Drop（出笔）** | 放置完成后将 `d.here` 更新为哪个引脚的坐标 | `.drop(名)` 换端子；`.hold()` 保持 `d.here` 不变 |

顺序串联时只需逐个 `d.add`：每颗元件的出笔（drop）自动成为下一颗的入笔，`d.here` 链式推进。跨支路或非串联走线时，用 `.at(锚点)` 显式指定位置，独立于 `d.here`。

### Placement（入笔）

- 未指定时，放置点为当前 `d.here`（上一元件的出笔，或画布初始位置）。
- `.at(p)` 将放置点设为坐标或另一元件的锚点（如 `M.drain`），相当于跳过默认入笔、改钉别处。
- `.anchor(name)` 指定以该引脚对齐到放置点（即「用哪个端子入笔」）。

典型默认放置锚点：二端元件为 `start`；`PFet` 为 `source`；`NFet` 为 `drain`。

### Drop（出笔）

元件加入画布后，`d.here` 更新为该元件 drop 端子的坐标。

**官方文档一般不标注 “drop”**，仅给出锚点名称。推断规则：

1. **二端元件**（`Element2Term`：电阻、电容、二极管、导线等）：默认 drop = **`end`**，即沿绘制方向的出口端（`.right()` 时在右端，`.down()` 时在下端）——与包围盒的「最右」「最下」无关，取决于绘制方向。
2. **多端元件**：默认 drop 由库内 `elmparams['drop']` 决定，需对照锚点或运行时验证。

常见多端默认（元件默认朝向）：

| 元件 | 默认 drop（出笔端子） | 说明 |
|------|----------------------|------|
| `PFet` | `drain` | θ = 0 时位于竖直沟道下端 |
| `NFet` | `source` | 同上，便于自上而下串联 |
| `PFet2` / `NFet2` | `end` | 二端件风格；`.right()` 时在右端 |
| `BjtNpn` | `collector` | 非沟道下端 |
| `Opamp` | `out` | 输出端 |
| `JFetN` / `JFetP` | `gate` | 栅极 |
| `Ground` / `Vdd` 等 | 连接点 | 出笔几乎不动，下一入笔仍在原处 |

可覆盖默认行为：

```python
d.add(elm.PFet().right().reverse().drop('gate'))  # 出笔改到栅极（下一默认入笔亦在此）
d.add(elm.Resistor().right().hold())              # 不出笔：d.here 保持入笔处
```

运行时核对：

```python
print(d.here)
print(m.source, m.drain, m.gate)
```

## 朝向：旋转与镜像

Placement / Drop 回答「从哪入、从哪出」；**朝向**回答「符号在纸面上怎么转」。`.right()` / `.up()` / `.left()` / `.down()` 设置元件旋转角 θ，参考系为画布 **x 正半轴**（θ = 0°）：

| 方法 | θ |
|------|---|
| `.right()` | 0° |
| `.up()` | 90° |
| `.left()` | 180° |
| `.down()` | 270° |

等价写法：`.theta(角度)`。

语义说明：

- 二端元件本地几何沿 **+x** 定义，故 `.right()` 表示主体沿 +x 延伸。
- `PFet` / `NFet` 在本地坐标中沟道沿 **−y** 方向。θ = 0 意为不旋转，沟道保持竖直，所以对这两种元件 `.right()` 的**外观是竖直沟道**——「right」只代表旋转角为 0°，并非表示沟道朝右延伸。
- `.reverse()` 为沿元件轴向的镜像（MOS 上常用于切换栅极左右）；`.flip()` 为另一方向翻转。二者与 θ 旋转相互独立。

改变 θ 后，drop 仍绑定同一命名端子，其在纸面上的方位会随旋转改变。

## 常用 API 接口

构造 `Element` 时常用的方法：

| 方法 | 作用 |
|------|------|
| `.right()` / `.left()` / `.up()` / `.down()` / `.theta(θ)` | 旋转 |
| `.at(p)` | 指定放置点 |
| `.anchor(name)` | 指定对齐引脚 |
| `.to(p)` | 连线终点（如 `Line`） |
| `.length(n)` | 长度 |
| `.label(...)` | 标注 |
| `.reverse()` / `.flip()` | 镜像 / 翻转 |
| `.drop(...)` / `.hold()` | 覆盖出笔端子 / 冻结 `d.here`（不出笔） |

非串联连接时，优先用已放置元件的锚点绝对坐标（`.at` / `.to`），不必依赖 `d.here` 顺序。

## 示例：CMOS 反相器

结构：电源侧 `PFet` 与地侧 `NFet` 串联，栅极共接 `IN`，中间节点引出 `OUT`。下面脚本把前述对象与规则串起来：`Drawing` 作画布，`PFet` / `NFet` / `Line` 等为 `Element`，串联靠入笔 / 出笔衔接，跨支路靠锚点 `.at` / `.to`。

{% include_code lang:python tech/ic/schemdraw-basics-01.py %}

```bash
python3 schemdraw-basics-01.py
```

生成的 CMOS 反相器原理图如下：

![CMOS 反相器原理图](/assets/images/tech/ic/schemdraw-basics-01.svg)

与上述概念的对应关系：

1. `elm.PFet().right().reverse()`：θ = 0（竖直沟道），`.reverse()` 将栅极置于左侧；默认自 `source` 入笔，出笔（drop）至 `drain`。
2. `elm.NFet(...).anchor('drain')`：以 `drain` 对齐当前笔尖（上一出笔），完成漏极侧串联。
3. `Line().at(MP.gate).to(MN.gate)` 与中点引出：基于锚点坐标布线，独立于画笔串联的入/出笔路径。
