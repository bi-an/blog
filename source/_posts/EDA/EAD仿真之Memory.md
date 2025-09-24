---
title: EDA 仿真之 Memory
date: 2025-08-20 22:25:49
categories: EDA
tags:
---

## Memory

Memory = 存储 + 访问逻辑

存储

- 在仿真里，memory 本质上就是一个数组（Array）或者向量（Vector），每个元素对应一个存储单元（bit/byte/word）。
- 例：一个 8 位 × 1024 深度的 RAM，可以在仿真里用 `uint8_t mem[1024];` 表示。

访问逻辑

- 读（Read）：根据地址返回对应的数据。
- 写（Write）：根据地址和写使能信号，将数据写入存储单元。
- 可能涉及 时序：同步（clock 边沿写入）或异步（立即生效）。

时序和延迟

- 在硬件里，memory 访问不是瞬间的：存在 读延迟、写延迟。
- 仿真时，可以用 延时事件 或 clock 边沿触发 来模拟。

## 仿真代码

功能说明

- 多端口读写：支持同时多个端口访问 memory
- 写冲突仲裁：写优先策略或延迟写，可扩展读优先 / 轮询
- 读延迟 pipeline：延迟由 read_delay 控制
- Burst / wrap-around：访问超出末尾自动回绕
- 简单 Cache/Tag：模拟命中 / 未命中
- 异步端口：不同端口调用 read/write 可在不同 tick，模拟异步时钟
- 随机 bit flip / SEU：1% 概率错误注入
- 统计与功耗估算基础：记录读写次数、命中数、平均延迟

{% include_code lang:cpp eda/memory/EdaMemoryFull.cpp %}

输出的 memory_power.csv 文件内容示例：

```
Cycle,DynamicPower,StaticPower,TotalPower
1,3,128,131
2,0,128,128
3,5,128,133
4,2,128,130
5,7,128,135
6,1,128,129
7,4,128,132
8,0,128,128
9,2,128,130
10,3,128,131
```

每列含义：

- Cycle：周期号
- DynamicPower：每周期动态功耗单位
- StaticPower：静态功耗单位（固定）
- TotalPower：总功耗单位


功耗分析

{% include_code lang:python eda/memory/report_power.py %}

### burst/multi-port 总线冲突

在多端口系统中，尤其是在使用总线结构的系统中，总线冲突（Bus contention）是一个常见的问题。总线冲突通常发生在多个设备尝试同时访问总线上的同一资源时。这种情况可能会导致数据损坏、系统性能下降或甚至系统崩溃。下面是一些解决和缓解总线冲突的策略：

1. 仲裁机制
仲裁是解决总线冲突的一种常用方法。它通过一个仲裁器（Arbiter）来决定哪个设备可以访问总线。常见的仲裁策略有：

优先级仲裁：根据预先设定的优先级顺序决定哪个设备可以访问总线。

轮询仲裁：轮流让每个设备访问总线。

基于请求的仲裁（如请求共享（Request-for-Shared, RFS）和请求独占（Request-for-Exclusive, RFE））：设备首先请求对资源的访问，然后根据请求的类型（共享或独占）来决定访问权限。

2. 分时复用
通过时间分割（Time Division Multiplexing, TDM）或频率分割（Frequency Division Multiplexing, FDM），可以允许多个设备在不同的时间或频率上使用总线，从而减少冲突。例如，可以使用时分多路复用将总线的不同时间段分配给不同的设备。

3. 编码和解码技术
使用特殊的编码和解码技术，如霍纳编码（Hornar code）或格雷码（Gray code），可以减少在总线上传输数据时的错误，并帮助检测和纠正数据冲突。

4. 总线锁定
在访问总线期间，通过总线锁定机制确保没有其他设备可以访问总线。这可以通过在总线上设置一个锁定信号来实现，该信号在访问期间保持激活状态。

5. 缓存和缓冲
为每个设备提供局部缓存或缓冲机制，可以减少对总线的直接访问次数，从而降低冲突的可能性。当一个设备需要与总线上的另一个设备通信时，它可以先将数据写入自己的缓存，然后再由缓存同步到总线上。

6. 使用更宽的总线
增加总线的宽度可以允许在同一时间内传输更多的数据，从而减少对总线的需求，降低冲突的可能性。

实施步骤
评估系统需求：确定哪些类型的设备将使用总线，以及它们对带宽的需求。

选择仲裁策略：根据设备的优先级和带宽需求选择合适的仲裁策略。

设计硬件：根据选定的策略设计硬件，包括添加仲裁器、缓存和适当的控制逻辑。

测试和优化：实施后进行系统测试，根据测试结果调整策略或硬件设计。

通过上述方法，可以有效管理和减少多端口系统中的总线冲突问题，提高系统的稳定性和性能。


### Cache Tag（缓存标记）

Cache Tag（缓存标记）是高速缓存（Cache）中的关键组成部分，用于存储数据在主存中的地址信息，以便快速定位数据位置。 ‌

核心功能
Tag字段存储了主存中数据的地址信息，当CPU访问主存时，首先通过Tag字段判断数据是否存在于Cache中。若存在，则直接从Cache读取；若不存在，则访问主存。 ‌

结构组成
- ‌Tag‌：记录数据在主存的地址信息。
- ‌Data‌：存储实际数据。
- ‌Valid Bit‌：标记数据是否有效。
- ‌Dir‌：目录信息，用于区分不同数据块。 ‌

应用场景

现代处理器通常采用多级Cache结构（如L1、L2、L3），其中Tag与Data共同构成Cache Line，用于快速访问和存储数据。例如，ARMv8-A架构的处理器包含独立的I-Cache和D-Cache，分别存储指令和数据。

Cache Tag 仿真代码

> FIXME: 该代码会 coredump 。

{% include_code lang:cpp eda/memory/cache/cache_simulator.h %}

{% include_code lang:cpp eda/memory/cache/cache_simulator.cpp %}

{% include_code lang:cpp eda/memory/cache/main.cpp %}

{% include_code lang:cpp eda/memory/cache/Makefile %}

