---
title: 为什么同一个多线程程序在两台机器上 Virtual Memory 差了 10 倍？
date: 2026-07-23 15:03:18
categories: memory
tags:
 - memory
 - thread
---

## 现象

同一多线程工具、同一版本、同一份输入，在两台机器上跑完后，日志里的峰值内存对不上：

| 机器 | Peak RSS | Peak Virt | Virt / RSS |
|------|----------|-----------|------------|
| 机器 A | 473 GB | 518 GB | 1.09 |
| 机器 B | 478 GB | 5485 GB | 11.5 |

RSS 只差约 5 GB，Virt 却差约 4967 GB（近 5 TB）。两次运行都正常结束（exit status 0），结果也正确。机器 B 多带了 `-mc`，并行度更高，但后文估算表明两机线程数接近（约 144），单凭执行路径解释不了 Virt 差这么多。

问题变成：日志里的 Virt 异常，是不是真出了内存问题？

## 排查

### 排除 OOM 与泄漏

| 机器 | freeSwap / totalSwap | 退出码 |
|------|----------------------|--------|
| 机器 A | 126 GB / 128 GB | 0 |
| 机器 B | 128 GB / 128 GB | 0 |

RSS 接近、swap 几乎没动、结果正确——更像监控数字异常，而不是进程把物理内存吃爆了。

### 对比 ulimit

| 机器 | `ulimit -s`（kbytes） | 约合每线程栈 |
|------|----------------------|--------------|
| 机器 A | 42768 | ~42 MB |
| 机器 B | 33554432 | 32 GB |

栈限制相差约 785 倍。若程序用的是默认线程栈，这条差异值得顺着查。

### 定量估算

输入相同、RSS 接近，可近似认为线程数 \(N\) 差不多。若线程走默认栈，Virt 差应主要来自「每线程栈预留」之差：

```text
ΔVirt          = 5485 − 518 = 4967 GB
Δstack/thread  = 32.000 − 0.041 = 31.959 GB
N（上界）      = 4967 / 31.959 ≈ 155
```

上界假设除线程栈外其余 Virt 完全相同。日志里还有更早的一段：多线程计算启动前、内存库刚加载完时，两机 Virt 已经差约 377 GB（A：Virt 252 GB / RSS 251 GB；B：Virt 629 GB / RSS 251 GB）。差额来源未完全确认，可能是分配器预留，也可能含少量早期线程。从总差里扣掉后再估：

```text
N（下界）= (4967 − 377) / 31.959 ≈ 144
```

\(N\) 大约在 144～155。两种估法都指向同一方向：**Virt 膨胀的主体，可以用 `ulimit -s` 造成的栈预留差解释。**

## 根因

未调用 `pthread_attr_setstacksize` 时，`pthread_create` 默认取 `RLIMIT_STACK`（即 `ulimit -s`）。内核为每个线程预留对应大小的虚拟地址区间：

1. 预留立刻计入进程 VSZ（日志里的 Virt）
2. 不立即分配物理页，故不计入 RSS
3. 线程真正写入栈时，才缺页映射物理页

按约 144 线程估算：

| 机器 | 每线程栈 | 线程栈合计计入 Virt |
|------|----------|---------------------|
| 机器 A | 42 MB | ≈ 6 GB |
| 机器 B | 32 GB | ≈ 4608 GB |

两机任务量和线程数接近，Virt 差的主因是每线程栈预留差了约 785 倍，而不是「机器 B 多干了很多活」。RSS 可以几乎不变：真正触达的页才进 RSS；未触达的栈预留只抬高 Virt。

### 题外话：32 位地址空间下的差异

上面「Virt 很大但仍能正常跑完」主要适用于 64 位进程。x86_64 用户态地址空间约 128 TB，本文约 5 TB 的 Virt 远没顶到天花板。

32 位用户态通常约 3 GB（Linux 默认约 3 GB / 1 GB 用户/内核分割）。这时过大的 `ulimit -s` 会很快耗尽地址空间：每多一个线程就多占数百 MB 虚拟地址，线程稍多，`pthread_create` 就会返回 `EAGAIN`（errno=11，Resource temporarily unavailable），程序直接失败。

用 `ulimit -v 3145728` 把虚拟地址空间限制为 3 GB，模拟 32 位上限，实测如下：

```text
# 异常场景：ulimit -s 262144（256 MB/线程）
ulimit -v 3145728; ulimit -s 262144; ./thread-stack-32bit-demo

ulimit -s (stack/thread) = 256 MB
ulimit -v (virtual addr) = 3072 MB
理论最大线程数 ≈ 3072 / 256 = 12

  [  0 threads]  Virt =      2 MB   RSS =     1 MB
  [  1 threads]  Virt =    258 MB   RSS =     1 MB   ← 每线程 +256 MB Virt
  [  2 threads]  Virt =    514 MB   RSS =     1 MB   ← RSS 始终只有 1 MB
  [  3 threads]  Virt =    770 MB   RSS =     1 MB
  ...
  [ 11 threads]  Virt =   2818 MB   RSS =     1 MB

pthread_create 在第 12 个线程时失败: Resource temporarily unavailable (errno=11, EAGAIN)
                                                                     ^^^^^^^^^^^^^^^^^^^^
                         虚拟地址空间耗尽，内核无法为新线程栈执行 mmap，返回 EAGAIN
成功创建线程数: 11

# 对照组：ulimit -s 8192（8 MB/线程，正常值）
ulimit -v 3145728; ulimit -s 8192; ./thread-stack-32bit-demo

  [  1 threads]  Virt =     10 MB   RSS =     1 MB   ← 每线程仅 +8 MB Virt
  [  2 threads]  Virt =     18 MB   RSS =     1 MB
  ...（可正常创建 300+ 个线程）
```

两组对比很清楚：RSS 全程只有约 1 MB（栈几乎没真正用起来），Virt 却按 `ulimit -s` 的步长往上加，直到地址空间耗尽报错。对 32 位程序，过大的 `ulimit -s` 是实质故障；对 64 位程序，同类配置多半只让日志里的 Virt「看起来吓人」。复现程序：

{% include_code lang:c title:thread-stack-32bit-demo.c tech/memory/virtual-memory-thread-stack-analysis-01.c %}

## 定位方法

这次是日志对比触发的个案。若以后再看到 Virt 远大于 RSS，仍建议先定位最大的映射段，再归因（文件 `mmap`、分配器预留、共享内存等都可能）。

```bash
cat /proc/<PID>/status | grep -E 'VmSize|VmRSS|VmSwap'
pmap -x <PID> | sort -k3 -rn | head -10
```

`pmap -x` 里：Kbytes 是段的虚拟大小（进 Virt），RSS 是驻留页，Mapping 标明类型（`[stack:TID]`、`[heap]`、路径、`[anon]` 等）。

本次这类场景下，`ulimit -s` 为 32 GB 时，排序结果通常类似：

```text
Address           Kbytes       RSS   Dirty Mode  Mapping
00007f1200000000  33554432      128       0 rw--- [stack:4312]
00007f2200000000  33554432       64       0 rw--- [stack:4313]
...（约 144 行 [stack:*]）
0000000001a00000   524288   263144   263144 rw--- [heap]
```

单段 Kbytes = 33554432 即约 32 GB 虚拟预留，RSS 往往只有几十到几百 KB。

- `[stack:N]` → 查 `ulimit -s` 和线程数
- 大块 `[anon]` → 查分配器或显式 `mmap`
- 具名路径 → 查映射文件大小

```bash
awk '/\[stack/{s=1} s && /^Size/{sum+=$2; s=0} END{print sum/1024 " MB"}' \
    /proc/<PID>/smaps
ulimit -s
cat /proc/<PID>/status | grep Threads
```

## 结论

| | 机器 A | 机器 B |
|---|--------|--------|
| `ulimit -s` | 42 MB | 32 GB |
| 线程数（默认栈） | ~144 | ~144 |
| 线程栈合计 Virt | ~6 GB | ~4608 GB |
| Peak RSS | 473 GB | 478 GB |
| Peak Virt | 518 GB | 5485 GB |
| 运行是否正常 | 是 | 是 |

**这次日志里的 Virt「异常」，不是泄漏，也不是跑挂了。** 两机默认线程栈都绑在 `RLIMIT_STACK` 上，而 `ulimit -s` 差了约 785 倍，多线程一叠加，Virt 就被放大到近 10 倍；RSS 与正确性可以保持正常。

可落地的收尾：

| 场景 | 做法 |
|------|------|
| 程序内工作线程 | 用 `pthread_attr_setstacksize` 设明确栈大小（如 8 MB） |
| 机器 / 集群默认 | `ulimit -s` 建议 8～64 MB，避免对多线程负载设 `unlimited` |
| 看日志 / 告警 | 多线程进程优先看 RSS（和 swap），不要单凭 Virt 判内存异常 |
| 跨机对比 | 同二进制 Virt 差很多时，先比双方 `ulimit -a` |
