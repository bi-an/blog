---
title: EDA 仿真之 Memory
date: 2025-08-20 22:25:49
categories: EDA
tags:
---

## memory

Memory = 存储 + 访问逻辑

存储

- 在仿真里，memory 本质上就是一个数组（Array）或者向量（Vector），每个元素对应一个存储单元（bit/byte/word）。
- 例：一个 8 位 × 1024 深度的 RAM，可以在仿真里用 uint8_t mem[1024]; 表示。

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


{% include_code lang:cpp eda/memory/AdancedEdaMemory.cpp %}

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

