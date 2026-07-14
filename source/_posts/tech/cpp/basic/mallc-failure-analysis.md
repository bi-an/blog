---
title: malloc(64 GiB) 返回 NULL 排查记录
date: 2026-07-14 11:56:13
categories: cpp
tags:
  - cpp
  - basic
---

## 一、现象

在两台 64 位 Linux 主机上运行同一段代码，结果截然不同：

|                  | 主机 A（RHEL 9.4，内核 5.14） | 主机 B（RHEL 8.10，内核 4.18） |
| ---------------- | ----------------------------- | ------------------------------ |
| `malloc(64 GiB)` | 返回 `NULL`，`errno = ENOMEM` | 返回有效指针，成功             |
| 物理内存         | 约 7.5 GiB                    | 约 3.0 TiB                     |
| Swap             | 约 8.0 GiB                    | 约 128 GiB                     |
| overcommit 模式  | mode 0（默认）                | mode 0（默认）                 |

直觉上，64 位进程拥有约 128 TiB 的虚拟地址空间，为什么请求区区 64 GiB 会返回"内存不足"？

---

## 二、背景：Linux 的"先承诺、后兑现"内存模型

理解这个问题，需要先搞清楚 Linux 的内存分配不是"直接给你物理内存"：

- **`malloc` 调用 `mmap`**：请求的块超过约 128 KiB 时，glibc 内部不通过 `brk` 扩堆，而是调用 `mmap`
  系统调用申请匿名内存。
- **承诺（commit）在先，物理页在后**：`mmap` 成功时，内核只是"承诺"了一段虚拟地址区间，物理页面要等
  到程序真正读写时才分配（缺页异常触发）。
- **承诺也有上限**：内核在 `mmap` 时会检查"这次的承诺量是否合理"，超过限制直接拒绝，返回 `ENOMEM`——
  即使此刻物理 RAM 还没用完。

因此 `ENOMEM` 不一定意味着"内存满了"，可能只是内核在承诺阶段预判"将来可能兑现不了"而提前拒绝。

---

## 三、排查过程

### 3.1 确认失败发生在哪一层：`malloc` 还是 `mmap`？

`malloc` 失败可能有两种情况：glibc 内部逻辑拒绝，或底层 `mmap` 系统调用已失败。先分层确认：

```bash
./probe_malloc_failure mmap
```

| 调用                       | 主机 A                 | 主机 B |
| -------------------------- | ---------------------- | ------ |
| `mmap(64 GiB)`（默认标志） | `MAP_FAILED`，errno=12 | 成功   |
| `malloc(64 GiB)`           | `NULL`，errno=12       | 成功   |

主机 A 上 `mmap` 系统调用本身已失败，`malloc` 返回 NULL 只是下游表现。**排查范围收缩至内核 `mmap` 路
径。**

---

### 3.2 假设：进程虚拟地址空间受到 `RLIMIT_AS` 限制

Linux 可以通过 `RLIMIT_AS`（address space limit，进程虚拟地址空间上限）限制单个进程可映射的虚拟地址
总量。内核在 `mmap_region()` 中的 `may_expand_vm()` 检查：若当前已映射量 + 本次请求 > 上限，拒绝并返
回 `ENOMEM`。

查看方式：

```bash
ulimit -v
cat /proc/self/limits | grep -i 'max address'
```

|             | 主机 A      | 主机 B      |
| ----------- | ----------- | ----------- |
| `RLIMIT_AS` | `unlimited` | `unlimited` |

两台主机均无限制，**排除此假设**。

---

### 3.3 假设：内核 overcommit 检查拒绝了本次请求

Linux 允许全系统"承诺"的虚拟内存总量超过物理 RAM，这叫**超量承诺（overcommit）**。策略由
`/proc/sys/vm/overcommit_memory` 控制，共三种模式：

| mode               | 值  | 行为                                                       |
| ------------------ | --- | ---------------------------------------------------------- |
| **启发式**（默认） | `0` | 单次 `mmap` 请求的页数 > 物理 RAM 页数 + Swap 页数，则拒绝 |
| **始终允许**       | `1` | 无条件同意所有分配                                         |
| **严格限制**       | `2` | 全系统累计承诺量 < 上限时才允许                            |

两台主机均为默认的 **mode 0**。其核心判断逻辑（Linux v5.14，`mm/util.c`）：

```c
// __vm_enough_memory()
if (sysctl_overcommit_memory == OVERCOMMIT_GUESS) {
    if (pages > totalram_pages() + total_swap_pages)
        goto error;   // 返回 -ENOMEM
}
```

**在主机 A 上代入数字：**

| 数据项                        | 值                                    |
| ----------------------------- | ------------------------------------- |
| 请求页数（64 GiB ÷ 4 KiB/页） | 16,777,216 页                         |
| `totalram_pages`（物理 RAM）  | 1,967,521 页                          |
| `total_swap_pages`（Swap）    | 2,097,151 页                          |
| **系统容量合计**              | **4,064,672 页（约 15.5 GiB）**       |
| 判断                          | 16,777,216 **>** 4,064,672 → **拒绝** |

**在主机 B 上代入数字：**

| 数据项                     | 值                                      |
| -------------------------- | --------------------------------------- |
| 系统容量合计（RAM + Swap） | 约 825,719,106 页（约 3.1 TiB）         |
| 判断                       | 16,777,216 **≪** 825,719,106 → **通过** |

这直接解释了两台主机的差异。

**用实验验证边界：**

```bash
./probe_malloc_failure threshold
```

```
15360 MiB: OK
15872 MiB: OK
15880 MiB: FAIL errno=12
16384 MiB: FAIL errno=12
65536 MiB: FAIL errno=12
```

| 请求大小   | 请求页数   | 与系统容量比较      | 实测结果 |
| ---------- | ---------- | ------------------- | -------- |
| 15,872 MiB | 4,063,232  | ≤ 4,064,672（通过） | 成功     |
| 15,880 MiB | 4,065,280  | > 4,064,672（超限） | 失败     |
| 64 GiB     | 16,777,216 | > 4,064,672（超限） | 失败     |

实测边界与公式完全吻合，**确认 mode 0 overcommit 检查是根因**。

> **内核版本说明**：`__vm_enough_memory()` 的 mode 0 实现随内核版本不同。Linux **5.14** 起 mode 0 只
> 做上述单次请求页数检查；**5.14 之前**（如主机 B 的 4.18）采用基于空闲页的估算公式，结论相同但计算
> 路径不同。查阅源码前须先用 `uname -r` 确认内核主版本号。

---

### 3.4 排除：mode 2 的累计承诺量限制

既然是 mode 0，顺手排除 mode 2 的逻辑：mode 2 下，全系统累计已承诺量（`Committed_AS`）超过上限
（`CommitLimit`）时才拒绝。两台主机均为 mode 0，这条路径根本不走。

用实验双重确认：

```bash
./probe_malloc_failure commit
```

```
before: Committed_AS = 6327800 kB
mmap(15872 MiB): OK   Committed_AS 6327800 -> 22580728 (delta 16252928 kB)
mmap(16 GiB):    FAIL  Committed_AS 6327800 -> 6327800  (delta 0 kB)
```

- `mmap(15872 MiB)` 成功后，`Committed_AS` 升至约 21.5 GiB，已超过 `CommitLimit`（约 11.8 GiB），却
  没有报错——证明 mode 2 限制未生效。
- `mmap(16 GiB)` 失败时，`Committed_AS` 完全没变——内核在拒绝时根本没有计入承诺量，说明失败发生在承诺
  记账之前（mode 0 的单次页数检查）。

**排除 mode 2 累计承诺量限制。**

---

### 3.5 排除：地址空间碎片化，找不到连续区域

`mmap(NULL, size)` 要求内核在进程虚拟地址空间中找到一段**连续**的、长度为 `size` 的未映射区域。如果
已有映射将地址空间切得过碎，即使未映射区域总量够，也可能找不到一段连续 64 GiB 的空闲区间。

**区分方法**：`MAP_NORESERVE` 标志会让内核**跳过 overcommit 承诺检查**，但不会跳过"寻找连续区域"这一
步。因此：

- 加了 `MAP_NORESERVE` 后 `mmap` **成功** → 连续区域是有的，失败只因承诺检查
- 加了 `MAP_NORESERVE` 后 `mmap` **仍失败** → 可能确实找不到连续区域

```bash
./probe_malloc_failure noreserve
```

| 请求大小 | 默认 `mmap`    | `mmap + MAP_NORESERVE` |
| -------- | -------------- | ---------------------- |
| 16 GiB   | 失败，errno=12 | **成功**               |
| 64 GiB   | 失败，errno=12 | **成功**               |

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
  ▼    ▼                   │         │
ENOMEM 通过                │         │
主机A  ──────────────────────┘         │
失败点                                 │
              ┌────────────────────────┘
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

| 项目     | 内容                                                                           |
| -------- | ------------------------------------------------------------------------------ |
| 直接原因 | Linux mode 0 overcommit 检查：单次 `mmap` 请求页数 > 物理 RAM 页数 + Swap 页数 |
| 内核函数 | `__vm_enough_memory()`（`mm/util.c`，Linux v5.14 L885–886）                    |
| 主机 A   | 64 GiB（16,777,216 页） > RAM + Swap（4,064,672 页，约 15.5 GiB） → ENOMEM     |
| 主机 B   | 64 GiB 仅占约 3.1 TiB 容量的 2%，检查通过                                      |
| 已排除   | 进程 VA 上限（`RLIMIT_AS`）、地址空间碎片化、mode 2 累计承诺量限制             |

---

## 六、测试程序

源码：`probe_malloc_failure.C`

### 编译与运行

```bash
cd ua/Debug/uart/src/test
g++ -O0 -o probe_malloc_failure probe_malloc_failure.C
./probe_malloc_failure mmap        # §3.1 定位失败层
./probe_malloc_failure threshold   # §3.3 验证 overcommit 边界
./probe_malloc_failure commit      # §3.4 排除 mode 2
./probe_malloc_failure noreserve   # §3.5 排除地址碎片化
```

### 预期输出对照

| 子命令      | 主机 A                                                                                       | 主机 B |
| ----------- | -------------------------------------------------------------------------------------------- | ------ |
| `mmap`      | mmap FAIL errno=12；malloc NULL                                                              | 均成功 |
| `threshold` | ≥15880 MiB 失败，≤15872 MiB 成功                                                             | 均成功 |
| `commit`    | 16 GiB 失败时 `Committed_AS` 不变；15872 MiB 成功后 `Committed_AS` 超 `CommitLimit` 也不报错 | 均成功 |
| `noreserve` | 默认失败；加 `MAP_NORESERVE` 成功                                                            | 均成功 |

### 源码

```cpp
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

namespace {

constexpr size_t kGiB = 1024ULL * 1024 * 1024;

static long read_meminfo_kb(const char* key)
{
    std::ifstream f("/proc/meminfo");
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(key) == 0) {
            long v = 0;
            sscanf(line.c_str(), "%*s %ld", &v);
            return v;
        }
    }
    return -1;
}

// §3.1：确认失败发生在 mmap 层而非 malloc 层
static void probe_mmap_vs_malloc()
{
    size_t sz = 64 * kGiB;
    errno = 0;
    void* mp = mmap(NULL, sz, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("mmap(64 GiB):   %s errno=%d (%s)\n",
           mp == MAP_FAILED ? "FAIL" : "OK", errno, strerror(errno));
    if (mp != MAP_FAILED) munmap(mp, sz);

    errno = 0;
    void* p = malloc(sz);
    printf("malloc(64 GiB): %s errno=%d (%s)\n",
           p ? "OK" : "NULL", errno, strerror(errno));
    if (p) free(p);
}

// §3.5：MAP_NORESERVE 绕过承诺检查，确认失败不是地址碎片化
static void probe_noreserve()
{
    for (size_t gb : {16ULL, 64ULL}) {
        size_t sz = gb * kGiB;
        errno = 0;
        void* d = mmap(NULL, sz, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        printf("%zu GiB default:   %s errno=%d\n",
               gb, d == MAP_FAILED ? "FAIL" : "OK", errno);
        if (d != MAP_FAILED) munmap(d, sz);

        errno = 0;
        void* n = mmap(NULL, sz, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        printf("%zu GiB NORESERVE: %s errno=%d\n",
               gb, n == MAP_FAILED ? "FAIL" : "OK", errno);
        if (n != MAP_FAILED) munmap(n, sz);
    }
}

// §3.3：探测 overcommit 检查的边界（RAM+Swap 附近逐步测试）
static void probe_threshold()
{
    for (int mb : {15360, 15872, 15880, 16384, 65536}) {
        size_t sz = static_cast<size_t>(mb) * 1024 * 1024;
        errno = 0;
        void* p = mmap(NULL, sz, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        printf("%5d MiB: %s errno=%d\n",
               mb, p == MAP_FAILED ? "FAIL" : "OK", errno);
        if (p != MAP_FAILED) munmap(p, sz);
    }
}

// §3.4：观察 Committed_AS 变化，排除 mode 2 累计限制
static void probe_committed()
{
    auto committed = []() { return read_meminfo_kb("Committed_AS:"); };

    long c0 = committed();
    size_t sz_ok = 15872ULL * 1024 * 1024;
    void* p1 = mmap(NULL, sz_ok, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("mmap(15872 MiB): %s  Committed_AS %ld -> %ld kB\n",
           p1 == MAP_FAILED ? "FAIL" : "OK", c0, committed());
    if (p1 != MAP_FAILED) munmap(p1, sz_ok);

    c0 = committed();
    void* p2 = mmap(NULL, 16 * kGiB, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("mmap(16 GiB):    %s  Committed_AS %ld -> %ld kB\n",
           p2 == MAP_FAILED ? "FAIL" : "OK", c0, committed());
    if (p2 != MAP_FAILED) munmap(p2, 16 * kGiB);
}

} // namespace

int main(int argc, char* argv[])
{
    const char* mode = argc > 1 ? argv[1] : "all";
    if (!strcmp(mode, "mmap")      || !strcmp(mode, "all")) probe_mmap_vs_malloc();
    if (!strcmp(mode, "threshold") || !strcmp(mode, "all")) probe_threshold();
    if (!strcmp(mode, "commit")    || !strcmp(mode, "all")) probe_committed();
    if (!strcmp(mode, "noreserve") || !strcmp(mode, "all")) probe_noreserve();
    return 0;
}
```

---

## 七、参考资料

| 资源                                             | 链接                                                                                    |
| ------------------------------------------------ | --------------------------------------------------------------------------------------- |
| `__vm_enough_memory`、`vm_commit_limit`（v5.14） | https://github.com/torvalds/linux/blob/v5.14/mm/util.c                                  |
| `__vm_enough_memory`（v4.18）                    | https://github.com/torvalds/linux/blob/v4.18/mm/util.c                                  |
| `mmap_region`、`accountable_mapping`（v5.14）    | https://github.com/torvalds/linux/blob/v5.14/mm/mmap.c                                  |
| overcommit 机制文档                              | https://github.com/torvalds/linux/blob/v5.14/Documentation/vm/overcommit-accounting.rst |
| `proc(5)` 手册                                   | https://man7.org/linux/man-pages/man5/proc.5.html                                       |

---

## 修订记录

| 日期    | 摘要                                                                       |
| ------- | -------------------------------------------------------------------------- |
| 2026-07 | 初版                                                                       |
| 2026-07 | 重构为「§2 可能原因与背景 → §3 排查实验」结构                              |
| 2026-07 | 重构为叙事结构，内联背景知识，以 ASCII 流程图替代步骤表格，去除 C1–C6 编号 |
