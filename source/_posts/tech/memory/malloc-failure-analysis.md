---
title: malloc(64 GiB) 返回 NULL 排查记录
date: 2026-07-14 11:56:13
categories: memory
tags:
 - memory
 - linux
---

## 一、现象

在两台 64 位 Linux 主机上运行同一段代码，结果截然不同：

| | 命令 | 主机 A（RHEL 9.4，内核 5.14） | 主机 B（RHEL 8.10，内核 4.18） |
|---|---|---|---|
| `malloc(64 GiB)` | `./probe_malloc_failure mmap`[^2] | 返回 `NULL`，`errno = ENOMEM` | 返回有效指针，成功 |
| 物理内存 | `grep MemTotal /proc/meminfo` | 约 7.5 GiB | 约 3.0 TiB |
| Swap | `grep SwapTotal /proc/meminfo` | 约 8.0 GiB | 约 128 GiB |
| overcommit 模式 | `cat /proc/sys/vm/overcommit_memory` | mode 0（默认） | mode 0（默认） |

直觉上，64 位进程拥有约 128 TiB 的虚拟地址空间，为什么请求区区 64 GiB 会返回"内存不足"？

---

## 二、背景：Linux 的"先承诺、后兑现"内存模型

理解这个问题，需要先搞清楚 Linux 的内存分配不是"直接给你物理内存"：

- **`malloc` 调用 `mmap`**：请求的块超过约 128 KiB 时，glibc 内部不通过 `brk` 扩堆，而是调用 `mmap` 系统调用申请匿名内存。
- **承诺（commit）在先，物理页在后**：`mmap` 成功时，内核只是"承诺"了一段虚拟地址区间，物理页面要等到程序真正读写时才分配（缺页异常触发）。
- **承诺也有上限**：内核在 `mmap` 时会检查"这次的承诺量是否合理"，超过限制直接拒绝，返回 `ENOMEM`——即使此刻物理 RAM 还没用完。

因此 `ENOMEM` 不一定意味着"内存满了"，可能只是内核在承诺阶段预判"将来可能兑现不了"而提前拒绝。

---

## 三、排查过程

### 3.1 确认失败发生在哪一层：`malloc` 还是 `mmap`？

`malloc` 失败可能有两种情况：glibc 内部逻辑拒绝，或底层 `mmap` 系统调用已失败。先分层确认：

{% include_code probe_malloc_failure.cpp:32-48 lang:cpp from:32 to:48 tech/memory/malloc-failure-analysis-01.cpp %}

```bash
# 配套测试程序见第六节，编译：
# g++ -O0 -o probe_malloc_failure probe_malloc_failure.cpp
./probe_malloc_failure mmap
```

| 调用 | 主机 A | 主机 B |
|------|--------|--------|
| `mmap(64 GiB)`（默认标志） | `MAP_FAILED`，errno=12 | 成功 |
| `malloc(64 GiB)` | `NULL`，errno=12 | 成功 |

主机 A 上 `mmap` 系统调用本身已失败，`malloc` 返回 NULL 只是下游表现。**排查范围收缩至内核 `mmap` 路径。**

---

### 3.2 假设：进程虚拟地址空间受到 `RLIMIT_AS` 限制

Linux 可以通过 `RLIMIT_AS`（address space limit，进程虚拟地址空间上限）限制单个进程可映射的虚拟地址总量。内核在 `mmap_region()` 中的 `may_expand_vm()` 检查：若当前已映射量 + 本次请求 > 上限，拒绝并返回 `ENOMEM`。

查看方式：

```bash
ulimit -v
cat /proc/self/limits | grep -i 'max address'
```

| | 主机 A | 主机 B |
|---|---|---|
| `RLIMIT_AS` | `unlimited` | `unlimited` |

两台主机均无限制，**排除此假设**。

---

### 3.3 假设：内核 overcommit 检查拒绝了本次请求

Linux 允许全系统"承诺"的虚拟内存总量超过物理 RAM，这叫**超量承诺（overcommit）**。策略由 `/proc/sys/vm/overcommit_memory` 控制，共三种模式：

| mode | 值 | 行为 |
|---|---|---|
| **启发式**（默认） | `0`（`OVERCOMMIT_GUESS`） | 单次 `mmap` 请求的页数 > 物理 RAM 页数 + Swap 页数，则拒绝（Linux v5.14及以后）[^1] |
| **始终允许** | `1`（`OVERCOMMIT_ALWAYS`） | 无条件同意所有分配 |
| **严格限制** | `2`（`OVERCOMMIT_NEVER`） | 全系统累计承诺量 < 上限时才允许 |

两台主机均为默认的 **mode 0**。其核心判断逻辑（Linux v5.14，[`mm/util.c` L884–891](https://github.com/torvalds/linux/blob/v5.14/mm/util.c#L884-L891)）：

```c
// __vm_enough_memory()
if (sysctl_overcommit_memory == OVERCOMMIT_GUESS) {
    if (pages > totalram_pages() + total_swap_pages)
        goto error;   // 返回 -ENOMEM
}
```

**在主机 A 上代入数字：**

| 数据项 | 值 |
|---|---|
| 请求页数（64 GiB ÷ 4 KiB/页） | 16,777,216 页 |
| `totalram_pages`（物理 RAM） | 1,967,521 页 |
| `total_swap_pages`（Swap） | 2,097,151 页 |
| **系统容量合计** | **4,064,672 页（约 15.5 GiB）** |
| 判断 | 16,777,216 **>** 4,064,672 → **拒绝** |

**在主机 B 上代入数字：**

| 数据项 | 值 |
|---|---|
| 系统容量合计（RAM + Swap） | 约 825,719,106 页（约 3.1 TiB） |
| 判断 | 16,777,216 **≪** 825,719,106 → **通过** |

这直接解释了两台主机的差异。

**用实验验证边界：**

{% include_code probe_malloc_failure.cpp:71-85 lang:cpp from:71 to:85 tech/memory/malloc-failure-analysis-01.cpp %}

```bash
./probe_malloc_failure threshold
```

```
15360 MiB: OK errno=0
15872 MiB: OK errno=0
15880 MiB: FAIL errno=12
16384 MiB: FAIL errno=12
65536 MiB: FAIL errno=12
```

| 请求大小 | 请求页数 | 与系统容量比较 | 实测结果 |
|---|---|---|---|
| 15,872 MiB | 4,063,232 | ≤ 4,064,672（通过） | 成功 |
| 15,880 MiB | 4,065,280 | > 4,064,672（超限） | 失败 |
| 64 GiB | 16,777,216 | > 4,064,672（超限） | 失败 |

实测边界与公式完全吻合，**确认 mode 0 overcommit 检查是根因**。

---

### 3.4 排除：mode 2 的累计承诺量限制

两台主机的 `overcommit_memory` 都是 **0**，本不该走到 mode 2。仍用实验确认当前失败**不是**「全系统累计承诺量超限」。

下面按 Linux **v5.14** 源码（[`mm/mmap.c` `accountable_mapping` / `mmap_region`](https://github.com/torvalds/linux/blob/v5.14/mm/mmap.c)、[`mm/util.c` `__vm_enough_memory`](https://github.com/torvalds/linux/blob/v5.14/mm/util.c#L872-L911)）画出承诺检查：

```
mmap_region()
  │
  ├─ accountable_mapping()？   // 私有可写且无 VM_NORESERVE
  │     ├─ 否 → 跳过 __vm_enough_memory（不进入承诺检查）
  │     └─ 是 → security_vm_enough_memory_mm()
  │                    └─ __vm_enough_memory(pages)
  │                    │
  │                  ① vm_acct_memory(pages)
  │                     （先把本次 pages 计入 Committed_AS）
  │                    │
  │                  ② 按 overcommit_memory 三选一（互斥，只走一条）
  │             ┌─────────────────────┼─────────────────────────┐
  │             │ == 0 GUESS          │ == 1 ALWAYS             │ == 2 NEVER
  │             │ （当前主机）         │                         │
  │             ▼                     ▼                         ▼
  │   pages >                     直接通过                  allowed = vm_commit_limit()
  │   totalram + total_swap？     return 0                  （再减去 admin/user reserve）
  │         │是      │否        （保留记账）                    │
  │         │        │                                          ▼
  │         │        │                                      vm_committed_as（已含本次）
  │         │        │                                      < allowed ？
  │         │        │                                       │否（超限）  │是（未超）
  │         ▼        │                                       ▼            │
  │    goto error    │                                  goto error        │
  │                  │                                                    │
  │                  └─────────────────────────┬──────────────────────────┘
  │                                            ▼
  │                                        return 0
  │                                     （保留①的记账）
  │                                    mmap 继续成功路径
  │
  │     goto error:
  │        ③ vm_unacct_memory(pages)  ← 仅失败路径
  │           回滚刚才的记账
  │           return -ENOMEM
```

简化版（只看主干）：

```
accountable_mapping？  // 私有可写且无 VM_NORESERVE
  ├─ 否 → 跳过承诺检查，mmap 继续（仍可能因其它原因失败）
  └─ 是 → 先记账，再按 overcommit_memory：
              ├─ 0 → 单次 pages > RAM+Swap？  是→失败(回滚) / 否→检查通过
              ├─ 1 → 始终通过承诺检查
              └─ 2 → Committed_AS 超限？  是→失败(回滚) / 否→检查通过
```

1. **mode 0 不检查 `CommitLimit`。**
2. **mode 2 不检查 `pages > RAM + Swap`。**
3. **记账是「先加；仅失败才回滚」**，故失败时 `Committed_AS` 净不变。

**第一步：读出 `CommitLimit`。**

```bash
grep CommitLimit /proc/meminfo
# CommitLimit: 12323644 kB  →  11.75 GiB
```

要排除 mode 2，需要选一个同时满足的请求大小：

1. **大于 `CommitLimit`（11.75 GiB）**——否则即使用 mode 2 也不会因这次请求触顶  
2. **不超过 mode 0 的拒绝条件**——即请求 ≤ RAM + Swap（主机 A 为 4,064,672 页 / 15.5 GiB），否则会先被 mode 0 拦下，看不清 mode 2

§3.3 实测 **15872 MiB（=15.5 GiB）** 正好落在该区间内（> 11.75 GiB 且 ≤ 15.5 GiB），用作本实验的主探测点。另用 **16 GiB（=16384 MiB）**（> 15.5 GiB）作对照，确认超 mode 0 边界时拒绝且不记账。

{% include_code probe_malloc_failure.cpp:87-121 lang:cpp from:87 to:121 tech/memory/malloc-failure-analysis-01.cpp %}

```bash
./probe_malloc_failure commit
```

```
CommitLimit: 12323644 KiB (11.75 GiB)
mmap(15872 MiB / 15.5 GiB): OK  Committed_AS 6953140 KiB (6.63 GiB) -> 23206068 KiB (22.13 GiB)
mmap(16384 MiB / 16 GiB): FAIL  Committed_AS 6953140 KiB (6.63 GiB) -> 6953140 KiB (6.63 GiB)
```

| 步骤 | 请求 | 与条件的关系 | 结果 | `Committed_AS` | 说明 |
|---|---|---|---|---|---|
| ① | 读 `CommitLimit` | 上限 = **11.75 GiB** | — | — | 划定「须大于此值」 |
| ② | 15872 MiB（=15.5 GiB） | 11.75 < 15.5 ≤ 15.5（RAM+Swap） | **成功** | 6.63 → **22.13 GiB** | 记账后已超过 `CommitLimit` 仍成功 → **不是 mode 2** |
| ③ | 16384 MiB（=16 GiB） | 16 > 15.5（超 mode 0 边界） | **失败** | 6.63 → **6.63 GiB**（不变） | 未记账即拒绝 → mode 0 单次页数检查 |

**排除 mode 2 累计承诺量限制。**

---

### 3.5 排除：地址空间碎片化，找不到连续区域

`mmap(NULL, size)` 要求内核在进程虚拟地址空间中找到一段**连续**的、长度为 `size` 的未映射区域。如果已有映射将地址空间切得过碎，即使未映射区域总量够，也可能找不到一段连续 64 GiB 的空闲区间。

**区分方法**：`MAP_NORESERVE` 标志会让内核**跳过 overcommit 承诺检查**，但不会跳过"寻找连续区域"这一步。因此：

- 加了 `MAP_NORESERVE` 后 `mmap` **成功** → 连续区域是有的，失败只因承诺检查
- 加了 `MAP_NORESERVE` 后 `mmap` **仍失败** → 可能确实找不到连续区域

{% include_code probe_malloc_failure.cpp:50-69 lang:cpp from:50 to:69 tech/memory/malloc-failure-analysis-01.cpp %}

```bash
./probe_malloc_failure noreserve
```

| 请求大小 | 默认 `mmap` | `mmap + MAP_NORESERVE` |
|---|---|---|
| 16 GiB | 失败，errno=12 | **成功** |
| 64 GiB | 失败，errno=12 | **成功** |

加上 `MAP_NORESERVE` 后立即成功，说明连续未映射区域完全够用，**排除地址空间碎片化**。

---

## 四、内核调用链全貌

下图展示 `malloc(64 GiB)` 到返回 NULL 的完整内核路径，以及各排查步骤对应的检查点：

```
                        malloc(64 GiB)
                              │
                    glibc: size > MMAP_THRESHOLD
                    → 转为 mmap() 系统调用
                              │
                        ┌─────▼─────┐
                        │ do_mmap() │
                        └─────┬─────┘
                              │
              ┌───────────────▼───────────────┐
              │  有 MAP_NORESERVE             │
              │  且 overcommit_memory ≠ 2?    │
              └──────┬────────────────┬───────┘
                   是│                │否
                     ▼                ▼
              置 VM_NORESERVE    vm_flags 不变
                     │                │
                     └────────┬───────┘
                              │
                        ┌─────▼──────┐
                        │mmap_region │
                        └─────┬──────┘
                              │
              ┌───────────────▼────────────────┐
              │  RLIMIT_AS 检查                │
              │  total_vm + pages > 上限?      │  §3.2 已排除
              └──────┬─────────────────┬───────┘
                  超限│                 │通过
                      ▼                 │
                  ENOMEM                │
                （VA 上限）              │
                              ┌─────────▼────────────┐
                              │  accountable_mapping()│
                              │  VM_NORESERVE 已置位? │
                              └──────┬────────────────┘
                         已置位（MAP_NORESERVE 路径）
                                     │
              ┌──────────────────────┤
              │ 未置位（默认路径）    │已置位（跳过承诺检查）
              ▼                      │
  __vm_enough_memory()               │
  mode 0:                            │
  pages > RAM + Swap?                │
  ┌────┬───────────────────┐         │
  │是  │否                 │         │
  ▼    ▼                  │         │
ENOMEM 通过                │         │
主机A  ────────────────────┘         │
失败点                               │
              ┌──────────────────────┘
              ▼
    get_unmapped_area()          §3.5 已排除
    查找连续未映射区域
    ┌────┬──────────────┐
    │未找到            │找到
    ▼                  ▼
  ENOMEM         建立 VMA
（碎片化）    malloc 返回有效指针
```

**主机 A 的默认路径**（止于 overcommit 检查）：

```
malloc(64 GiB)
  └─> glibc: mmap(64 GiB)                  ← 无 MAP_NORESERVE
       └─> do_mmap()                        ← VM_NORESERVE 未置位
            └─> mmap_region()
                 └─> accountable_mapping() == true
                      └─> __vm_enough_memory()
                           └─> 16,777,216 > 4,064,672 → ENOMEM
  malloc 返回 NULL
```

**`MAP_NORESERVE` 路径**（绕过 overcommit 检查，主机 A 也成功）：

```
mmap(64 GiB, MAP_NORESERVE)
  └─> do_mmap()                             ← VM_NORESERVE 置位
       └─> mmap_region()
            └─> accountable_mapping() == false   ← 跳过承诺检查
                 └─> get_unmapped_area()          ← 找到连续区域
                      └─> 建立 VMA，返回地址
```

---

## 五、根因总结

| 项目 | 内容 |
|---|---|
| 直接原因 | Linux mode 0 overcommit 检查：单次 `mmap` 请求页数 > 物理 RAM 页数 + Swap 页数 |
| 内核函数 | `__vm_enough_memory()`（`mm/util.c`，Linux v5.14 L885–886） |
| 主机 A | 64 GiB（16,777,216 页） > RAM + Swap（4,064,672 页，约 15.5 GiB） → ENOMEM |
| 主机 B | 64 GiB 仅占约 3.1 TiB 容量的 2%，检查通过 |
| 已排除 | 进程 VA 上限（`RLIMIT_AS`）、地址空间碎片化、mode 2 累计承诺量限制 |

---

## 六、测试程序

### 编译与运行

```bash
# 源码展示名：probe_malloc_failure.cpp
g++ -O0 -o probe_malloc_failure probe_malloc_failure.cpp
./probe_malloc_failure mmap        # §3.1 定位失败层
./probe_malloc_failure threshold   # §3.3 验证 overcommit 边界
./probe_malloc_failure commit      # §3.4 排除 mode 2
./probe_malloc_failure noreserve   # §3.5 排除地址碎片化
```

### 预期输出对照

| 子命令 | 主机 A | 主机 B |
|---|---|---|
| `mmap` | mmap FAIL errno=12；malloc NULL | 均成功 |
| `threshold` | ≥15880 MiB 失败，≤15872 MiB 成功 | 均成功 |
| `commit` | 16 GiB 失败时 `Committed_AS` 不变；15872 MiB 成功后 `Committed_AS` 超 `CommitLimit` 也不报错 | 均成功 |
| `noreserve` | 默认失败；加 `MAP_NORESERVE` 成功 | 均成功 |

### 源码

{% include_code probe_malloc_failure.cpp lang:cpp tech/memory/malloc-failure-analysis-01.cpp %}

---

## 七、参考资料

| 资源 | 链接 |
|---|---|
| `__vm_enough_memory`、`vm_commit_limit`（v5.14） | https://github.com/torvalds/linux/blob/v5.14/mm/util.c |
| `__vm_enough_memory`（v4.18） | https://github.com/torvalds/linux/blob/v4.18/mm/util.c |
| `mmap_region`、`accountable_mapping`（v5.14） | https://github.com/torvalds/linux/blob/v5.14/mm/mmap.c |
| overcommit 机制文档 | https://github.com/torvalds/linux/blob/v5.14/Documentation/vm/overcommit-accounting.rst |
| `proc(5)` 手册 | https://man7.org/linux/man-pages/man5/proc.5.html |

[^1]: 此行为为 Linux **5.14 及以后**的实现（[`mm/util.c` L885–886, v5.14](https://github.com/torvalds/linux/blob/v5.14/mm/util.c#L885-L886)）。5.14 之前（如本文主机 B 的 4.18）mode 0 改用基于系统空闲页的启发式估算（[`mm/util.c`, v4.18](https://github.com/torvalds/linux/blob/v4.18/mm/util.c)），公式不同，但对本次场景的结论一致。排查时须先用 `uname -r` 确认内核主版本号再对照源码。

[^2]: 配套测试程序 `probe_malloc_failure.cpp` 见[第六节](#六、测试程序)。
