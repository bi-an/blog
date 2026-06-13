---
categories: cpp
date: 2025-11-26 10:12:33
tags:
- cpp
- optimize
- performance
title: 性能分析工具
---

## 性能分析工具

| 工具                             |                   类型/定位                    | attach 支持 |                    分析范围                     |                             特点                              |               与 perf 的关系               |                开源/商业授权                 |            适用人群             |
| -------------------------------- | :--------------------------------------------: | :---------: | :---------------------------------------------: | :-----------------------------------------------------------: | :----------------------------------------: | :------------------------------------------: | :-----------------------------: |
| Oracle Performance Analyzer (PA) | 企业级性能分析器，Oracle Developer Studio 套件 |     ✅      | 用户态程序 (C/C++/Java/Fortran/Scala)，并行应用 |         GUI 丰富，低开销采样，函数/源代码/指令级分析          |                  独立工具                  |      商业软件 (Oracle Developer Studio)      |   企业/科研环境下的应用开发者   |
| perf                             |           Linux 内核自带性能分析框架           |     ✅      |           用户态 + 内核态 + 硬件事件            | 功能全面，支持调用链、硬件计数器，生态丰富 (eBPF, flamegraph) |                  基础框架                  |          开源 (GPL，Linux 内核自带)          |    系统工程师，底层性能调优     |
| gprofng                          |            GNU 新一代应用性能分析器            |     ✅      |      用户态程序 (C/C++/Java/Scala)，跨平台      |      基于实验目录，支持 GUI，跨语言，继承 Oracle PA 思路      |                  独立工具                  |              开源 (GNU 工具链)               |   应用开发者，跨平台性能优化    |
| gprof                            |          经典 GNU 性能分析器 (1980s)           |     ❌      |               用户态程序 (C/C++)                |  需编译时 -pg，生成 gmon.out，功能有限，不支持多线程/attach   |                  独立工具                  |              开源 (GNU 工具链)               |  学术/教学场景，简单函数级分析  |
| Intel PMU Tools                  |        Intel 提供的硬件性能计数器工具集        |     ✅      | 硬件事件 (CPU pipeline、cache、branch、memory)  |         基于 PMU，提供 top-down 分析，解释微架构瓶颈          | 依赖 perf 收集数据，属于 perf 的增强解释层 |  开源 (MIT/Apache 许可，Intel GitHub 提供)   |   系统/性能工程师，硬件级调优   |
| Intel Advisor                    |      高级性能优化工具 (Intel oneAPI 套件)      |     ✅      |           用户态程序，尤其是 HPC 应用           |      提供矢量化分析、内存访问优化、并行化建议，GUI 支持       |                  独立工具                  |   商业软件 (Intel oneAPI 套件，部分免费版)   |    HPC 开发者，科学计算优化     |
| Intel VTune Profiler             |                 深度性能分析器                 |     ✅      |      CPU/GPU/FPGA、内存、线程、I/O、微架构      |    微架构级剖析，支持 cache、pipeline、分支预测，功能全面     |        独立工具，但可结合 perf 数据        | 商业软件 (Intel oneAPI 套件，提供免费社区版) | 系统/性能工程师，硬件与应用优化 |
| Linaro MAP (Arm Forge MAP)       |             HPC 并行应用性能分析器             |     ✅      |        MPI、OpenMP、UPC 等大规模并行应用        |  低开销采样，跨节点整体分析，源代码行级耗时展示，GUI 可视化   |                  独立工具                  |     商业软件 (Arm Forge 套件，需许可证)      |   HPC 开发者，超级计算机环境    |

**差异总结**

- Oracle PA / gprofng：应用层性能分析，强调 GUI 和跨语言支持。
- perf：系统级通用工具，是底层框架。
- gprof：老工具，功能有限，不支持 attach。
- Intel PMU Tools：基于 perf，提供 Intel CPU 微架构解释，是 perf 的增强解释层。
- Intel Advisor：智能优化建议工具，适合 HPC 和科学计算。
- Intel VTune Profiler：深度剖析工具，能揭示硬件微架构瓶颈，适合单节点和复杂应用。
- Linaro MAP：专为 HPC 并行应用设计，低开销，跨节点整体性能分析。

**结论**

- 应用层性能分析：Oracle PA、gprofng
- 系统/内核级调优：perf
- 硬件级微架构分析：Intel PMU Tools、Intel VTune Profiler
- 智能优化建议 (HPC)：Intel Advisor
- 大规模并行应用分析 (HPC)：Linaro MAP

👉 可以把它们看作一个层次结构：

- gprof → gprofng/Oracle PA → perf → Intel PMU Tools/VTune → Advisor/MAP
- 逐步从函数级分析 → 应用性能 → 系统级 → 硬件级 → HPC 并行整体优化。

### Performance Analyzer

Oracle Developer Studio 提供了多种工具：Performance Analyzer、Thread Analyzer。

> [Performance Analyzer 官方文档](https://docs.oracle.com/cd/E77782_01/html/E77798/afagg.html#OSSPAgrkam)

#### 收集数据

使用 `collect` 命令收集数据
（[官方文档](https://docs.oracle.com/cd/E77782_01/html/E77798/afadn.html#scrolltoc)）。

```bash
collect collect-options program program-arguments
```

#### 开始性能分析

使用 `analyzer` 命令进行性能分析
（[官方文档](https://docs.oracle.com/cd/E77782_01/html/E77798/afafs.html#scrolltoc)）。

```bash
analyzer [control-options] [experiment | experiment-list]
```

例如：

```bash
analyzer -c test.1.er test.4.er
```

### Thread Analyzer

> [Thread Analyzer 官方文档](https://www.oracle.com/application-development/technologies/developerstudio-features.html#thread-analyzer-tab)

## 计时

### 计时工具 `time`

```bash
$ /usr/bin/time -p ls
```

Or,

```bash
$ time ls
```

其中（参
考[链接](https://ostechnix.com/how-to-find-the-execution-time-of-a-command-or-process-in-linux/)），

```bash
$ type -a time
time is a shell keyword
time is /usr/bin/time
```

### 软件和硬件定时器

https://weedge.github.io/perf-book-cn/zh/chapters/2-Measuring-Performance/2-5_SW_and_HW_Timers_cn.html
