---
title: NUMA节点详解
date: 2025-12-02 13:44:25
categories:
tags:
    - performance
    - numa
---

## 1. NUMA概念

NUMA（Non-Uniform Memory Access，非统一内存访问）是一种计算机内存设计架构，主要用于多处理器系统中。

在NUMA架构中：
- 每个CPU处理器都有本地内存（Local Memory），访问速度较快
- 每个CPU也可以访问其他CPU的内存（Remote Memory），但访问速度较慢
- 多个CPU和其本地内存组成一个NUMA节点（NUMA Node）
- 系统通过NUMA拓扑来管理内存和CPU的分配

**NUMA架构的优势：**
- 减少内存访问延迟（本地内存访问更快）
- 提高系统整体性能
- 支持更大规模的多处理器系统

**NUMA架构的挑战：**
- 需要合理分配进程和内存到对应的NUMA节点
- 跨节点访问内存会带来性能损失
- 需要应用程序或系统管理员进行优化

## 2. CPU亲和性概念

**CPU亲和性（CPU Affinity）**是指将进程或线程绑定到特定的CPU核心上运行的机制。通过设置CPU亲和性，可以：

1. **提高缓存命中率**：进程始终运行在同一个CPU核心上，可以更好地利用CPU的L1/L2/L3缓存
2. **减少进程迁移开销**：避免进程在不同CPU核心间频繁迁移带来的性能损失
3. **资源隔离**：将不同进程绑定到不同CPU核心，实现资源隔离和负载均衡

**CPU亲和性的类型：**
- **硬亲和性（Hard Affinity）**：强制绑定，进程/线程**只能**运行在指定的CPU核心上，调度器会严格遵循这个限制
  - 例如：如果绑定到CPU 0，进程/线程就**只能**调度到CPU 0上运行，**不能**调度到CPU 1或其他CPU核心上
  - 通过 `sched_setaffinity()`、`taskset`、`numactl --cpunodebind`、`cgroup cpuset.cpus` 等方法设置
- **软亲和性（Soft Affinity）**：偏好设置，系统会尽量将进程调度到指定的CPU核心，但不强制
  - 这是调度器的默认行为，在没有显式设置CPU亲和性时，调度器会尽量保持进程在同一个CPU核心上运行

**注意事项：**
- CPU亲和性设置会影响操作系统的调度器行为
- 过度绑定可能导致CPU负载不均衡
- 即使通过硬亲和性强制绑定到单个CPU核心，进程仍可能创建多个线程，这些线程会在该CPU核心上通过时间片轮转的方式轮流调度执行

**并行库适配说明：**

CPU绑定（如 `taskset`、`sched_setaffinity`、`numactl --cpunodebind`）是**硬亲和性**（强制性的），能有效限制并行库的线程数。例如，TBB（Threading Building Blocks）在初始化时会根据操作系统报告的硬件资源（通过 `sysconf(_SC_NPROCESSORS_ONLN)` 等API）来决定线程池大小和调度策略。如果使用 `taskset` 或 `numactl` 将进程绑定到部分CPU核心，TBB只会看到这些CPU核心，从而自动调整线程池大小。这意味着通过限制可见的CPU资源，可以间接控制并行库的行为。

**NUMA架构下CPU亲和性的特殊性：**

在NUMA架构系统中，CPU亲和性具有特殊的重要性：

1. **内存访问性能依赖CPU位置**：
   - 进程运行在节点A的CPU上，如果内存分配在节点B，会产生远程内存访问，性能显著下降
   - 因此，NUMA架构下的CPU亲和性设置必须配合内存分配策略，才能获得最佳性能

2. **节点级别的CPU绑定**：
   - 在NUMA系统中，通常以NUMA节点为单位进行CPU绑定，而不是单个CPU核心
   - 绑定到某个NUMA节点的CPU意味着可以使用该节点的所有CPU核心，同时配合该节点的本地内存

3. **CPU亲和性与内存策略的协同**：
   - 仅设置CPU亲和性而不设置内存策略可能导致远程内存访问
   - NUMA优化需要同时考虑CPU绑定和内存分配策略

**总结**：NUMA架构是CPU亲和性在多处理器系统中的特殊应用场景。在NUMA系统中，CPU亲和性不仅影响CPU调度，还直接影响内存访问性能，因此需要与内存分配策略协同使用。

## 3. CPU亲和性的设置

### 方法对比总览

| 方法 | 易用性 | NUMA感知 | 内存控制 | CPU绑定类型 | 适用场景 |
|------|--------|----------|----------|------------|----------|
| **taskset** | ⭐⭐⭐⭐⭐ | ❌ | ❌ | ✅ 硬亲和性 | 简单CPU绑定 |
| **sched_setaffinity** | ⭐⭐⭐ | ❌ | ❌ | ✅ 硬亲和性 | 程序内部控制 |
| **cgroup** | ⭐⭐ | ✅ | ✅ | ✅ 硬亲和性 | 系统级资源管理 |
| **numactl** | ⭐⭐⭐⭐ | ✅ | ✅ | ✅ 硬亲和性 | NUMA优化（详见NUMA章节） |

### 方法一：taskset（通用CPU绑定）

`taskset` 是一个用于设置或查看进程CPU亲和性的命令行工具，适用于所有系统（包括非NUMA系统）。

**基本用法：**

```bash
# 查看进程的CPU亲和性
taskset -p <pid>
taskset -cp <pid>  # 更易读的格式

# 设置进程的CPU亲和性（使用CPU掩码）
taskset -p 0x3 <pid>  # 绑定到CPU 0和1（二进制：11）

# 使用CPU列表格式
taskset -cp 0,1,2,3 <pid>  # 绑定到CPU 0,1,2,3

# 启动新进程并设置CPU亲和性
taskset -c 0-3 ./your_program  # 绑定到CPU 0-3
taskset -c 0,2,4,6 ./your_program  # 绑定到CPU 0,2,4,6
```

**taskset的特点：**
- ✅ 简单易用，命令行工具
- ✅ 直接指定CPU编号，精确控制
- ✅ CPU绑定是**硬亲和性**（强制绑定）
- ❌ 不感知NUMA拓扑结构
- ❌ 无法控制内存分配策略

**适用场景：**
- 非NUMA系统（单节点系统）
- 只需要简单的CPU绑定，不关心内存位置
- 系统没有NUMA架构

### 方法二：sched_setaffinity系统调用

`sched_setaffinity()` 是Linux系统提供的系统调用，用于设置进程或线程的CPU亲和性。这是 `taskset` 命令的底层实现。

**C语言示例：**

{% include_code perf/numa/sched_setaffinity.c %}

**多线程示例：**

{% include_code perf/numa/sched_setaffinity_threads.c %}

编译命令：
```bash
gcc -o sched_affinity sched_affinity.c -lpthread
```

**API说明：**
- `sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *mask)`：设置进程的CPU亲和性
  - `pid = 0`：设置当前进程
  - `pid > 0`：设置指定进程
- `sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask)`：获取进程的CPU亲和性
- `pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *mask)`：设置线程的CPU亲和性
- `pthread_getaffinity_np(pthread_t thread, size_t cpusetsize, cpu_set_t *mask)`：获取线程的CPU亲和性

**sched_setaffinity的特点：**
- ✅ 系统级API，功能强大
- ✅ 可以在程序内部精确控制CPU绑定
- ✅ 支持进程和线程级别的绑定
- ✅ CPU绑定是**硬亲和性**（强制绑定）
- ❌ 需要编写代码，不如命令行工具方便
- ❌ 不感知NUMA拓扑

**使用场景：**
- 需要在程序运行时动态调整CPU绑定
- 需要为不同线程设置不同的CPU亲和性
- 需要精确控制CPU绑定的应用程序

### 方法三：cgroup（系统级资源管理）

cgroup是Linux内核提供的资源管理机制，可以通过cgroup v1或cgroup v2来限制进程组的CPU使用。

**cgroup v1方法：**

```bash
# 创建cgroup
sudo mkdir -p /sys/fs/cgroup/cpuset/mygroup

# 设置可用的CPU核心（例如CPU 0-3）
echo "0-3" | sudo tee /sys/fs/cgroup/cpuset/mygroup/cpuset.cpus

# 设置内存节点（NUMA节点，如果系统支持）
echo "0" | sudo tee /sys/fs/cgroup/cpuset/mygroup/cpuset.mems

# 将进程添加到cgroup
echo <pid> | sudo tee /sys/fs/cgroup/cpuset/mygroup/cgroup.procs

# 或者启动新进程
sudo cgexec -g cpuset:mygroup ./your_program
```

**cgroup v2方法：**

```bash
# 挂载cgroup v2（如果未挂载）
sudo mount -t cgroup2 none /sys/fs/cgroup2

# 创建cgroup
sudo mkdir -p /sys/fs/cgroup2/mygroup

# 设置CPU范围（例如CPU 0-3）
echo "0-3" | sudo tee /sys/fs/cgroup2/mygroup/cpuset.cpus

# 设置内存节点（如果系统支持）
echo "0" | sudo tee /sys/fs/cgroup2/mygroup/cpuset.mems

# 将进程添加到cgroup
echo <pid> | sudo tee /sys/fs/cgroup2/mygroup/cgroup.procs
```

**systemd使用cgroup：**

```bash
# 创建systemd服务单元文件
sudo systemd-run --unit=myapp --scope \
    --property=CPUSetCPUs=0-3 \
    --property=CPUSetMems=0 \
    ./your_program
```

**cgroup的特点：**
- ✅ 系统级资源管理，功能强大
- ✅ 支持NUMA感知（可以设置内存节点）
- ✅ 可以同时管理CPU和内存
- ✅ 支持进程组管理
- ✅ **硬亲和性**（强制绑定），保证进程组只在指定CPU上运行
- ⚠️ 需要root权限
- ⚠️ 配置相对复杂

**使用场景：**
- 容器化环境（Docker、Kubernetes等）
- 需要同时管理多个进程的资源分配
- 需要持久化的资源限制配置
- 系统级资源管理需求

## 4. NUMA的设置

### 查询和监控NUMA信息

#### 1. 查看NUMA节点拓扑

使用 `numactl --hardware` 命令查看NUMA硬件信息：

```bash
$ numactl --hardware
available: 2 nodes (0-1)
node 0 cpus: 0 1 2 3 8 9 10 11
node 0 size: 16384 MB
node 0 free: 1024 MB
node 1 cpus: 4 5 6 7 12 13 14 15
node 1 size: 16384 MB
node 1 free: 2048 MB
node distances:
node   0   1
  0:  10  21
  1:  21  10
```

**结果说明：**
- `available: 2 nodes (0-1)`: 系统有2个NUMA节点，编号为0和1
- `node 0 cpus: 0 1 2 3 8 9 10 11`: 节点0包含的CPU核心编号
- `node 0 size: 16384 MB`: 节点0的内存大小（16GB）
- `node 0 free: 1024 MB`: 节点0的可用内存
- `node distances`: 节点间访问距离矩阵
  - 节点0到节点0的距离是10（本地访问）
  - 节点0到节点1的距离是21（远程访问，距离越大性能越差）

#### 2. 查看CPU和NUMA节点的映射关系

使用 `lscpu` 命令：

```bash
$ lscpu | grep -i numa
NUMA node(s):          2
NUMA node0 CPU(s):     0-3,8-11
NUMA node1 CPU(s):     4-7,12-15
```

#### 3. 查看NUMA统计信息

使用 `numastat` 查看NUMA统计信息：

```bash
$ numastat
                           node0           node1
numa_hit             12345678901     23456789012
numa_miss              123456789       234567890
numa_foreign           234567890       123456789
interleave_hit             12345           12345
local_node           12345678901     23456789012
other_node              123456789       234567890
```

#### 4. 查看进程的NUMA策略

使用 `numactl --show` 查看当前进程的NUMA策略：

```bash
$ numactl --show
policy: default
preferred node: current
physcpubind: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
cpubind: 0 1
nodebind: 0 1
membind: 0 1
```

#### 5. 查看系统NUMA节点详细信息

查看 `/sys/devices/system/node/` 目录：

```bash
$ ls /sys/devices/system/node/
node0  node1

$ cat /sys/devices/system/node/node0/cpulist
0-3,8-11

$ cat /sys/devices/system/node/node0/meminfo
Node 0 MemTotal:       16777216 kB
Node 0 MemFree:         1048576 kB
Node 0 MemUsed:       15728640 kB
```

### numactl：NUMA架构下的CPU亲和性和内存策略工具

`numactl` 是NUMA架构优化的首选工具，可以同时控制CPU和内存分配策略。

**基本用法：**

```bash
# 将进程绑定到节点0的CPU和内存（CPU是硬亲和性，内存是强制绑定）
numactl --cpunodebind=0 --membind=0 ./your_program

# 使用节点0的CPU（硬亲和性），但允许使用所有节点的内存
numactl --cpunodebind=0 ./your_program

# 使用节点0和1的CPU（硬亲和性），优先使用节点0的内存（偏好设置）
numactl --cpunodebind=0,1 --preferred=0 ./your_program

# 只绑定内存到节点0（强制绑定），CPU可以使用所有节点
numactl --membind=0 ./your_program

# 交错分配内存到多个节点
numactl --interleave=all ./your_program
```

**numactl选项说明：**

| 选项 | CPU绑定类型 | 内存绑定类型 | 说明 |
|------|------------|------------|------|
| `--cpunodebind=<nodes>` | ✅ **硬亲和性（强制绑定）** | - | 进程只能运行在指定节点的CPU上 |
| `--membind=<nodes>` | - | ✅ **强制绑定** | 内存只能从指定节点分配 |
| `--preferred=<node>` | - | ⚠️ **偏好设置** | 优先从指定节点分配内存，但允许从其他节点分配 |
| `--interleave=<nodes>` | - | ⚠️ **交错分配** | 在指定节点间轮询分配内存 |
| `--localalloc` | - | ⚠️ **本地优先** | 优先在本地节点分配内存 |

**详细说明：**
- `--cpunodebind=<nodes>`：将进程绑定到指定NUMA节点的CPU（硬亲和性，强制绑定）
  - 示例：`--cpunodebind=0` 绑定到节点0的所有CPU
  - 示例：`--cpunodebind=0,1` 绑定到节点0和1的所有CPU
- `--membind=<nodes>`：设置内存分配策略为强制绑定到指定节点（强制）
  - 示例：`--membind=0` 内存只能从节点0分配
  - 如果节点0内存不足，进程可能无法运行
- `--preferred=<node>`：设置内存分配偏好节点（偏好设置）
  - 示例：`--preferred=0` 优先从节点0分配内存
  - 如果节点0内存不足，允许从其他节点分配
- `--interleave=<nodes>`：在多个节点间交错分配内存
  - 示例：`--interleave=0,1` 在节点0和1间轮询分配
  - 适用于需要均匀使用多个节点内存的场景

**taskset与numactl的区别：**

| 特性 | taskset | numactl |
|------|---------|---------|
| **主要功能** | CPU亲和性设置 | NUMA节点和内存策略管理 |
| **CPU绑定** | ✅ 支持（基于CPU编号） | ✅ 支持（基于NUMA节点） |
| **内存管理** | ❌ 不支持 | ✅ 支持（内存节点绑定） |
| **NUMA感知** | ❌ 不感知NUMA拓扑 | ✅ 完全NUMA感知 |
| **使用场景** | 简单的CPU绑定需求 | NUMA架构优化 |

**关键区别：**
1. **CPU绑定方式不同**：
   - `taskset`：直接指定CPU编号（如CPU 0, 1, 2, 3）
   - `numactl`：基于NUMA节点指定（如节点0，自动包含该节点的所有CPU）

2. **内存管理能力**：
   - `taskset`：**无法控制内存分配**，进程可能从任意NUMA节点分配内存
   - `numactl`：可以控制内存分配策略，确保内存分配在特定NUMA节点

3. **NUMA架构优化**：
   - `taskset`：在NUMA系统中，即使绑定了CPU，内存仍可能从远程节点分配，导致性能问题
   - `numactl`：可以同时绑定CPU和内存，确保本地内存访问，获得最佳性能

**实际示例对比：**

```bash
# 使用taskset：只绑定CPU，不控制内存
taskset -c 0-3 ./program
# 问题：CPU在节点0，但内存可能从节点1分配（远程访问，性能差）

# 使用numactl：同时绑定CPU和内存
numactl --cpunodebind=0 --membind=0 ./program
# 优势：CPU和内存都在节点0（本地访问，性能好）
```

### NUMA函数和命令对比

下表列出了常用的NUMA相关函数和命令，以及它们的作用和绑定类型：

| 函数/命令 | 作用 | 绑定类型 | 说明 |
|----------|------|---------|------|
| `numa_alloc_onnode()` | 在指定NUMA节点分配内存 | ✅ **强制保证** | 内存一定分配在指定节点上，失败返回NULL |
| `numa_sched_setaffinity()` | 设置进程/线程的CPU亲和性 | ✅ **硬亲和性** | 进程/线程只能运行在指定的CPU核心上 |
| `numactl --cpunodebind` | 绑定进程到指定节点的CPU | ✅ **硬亲和性** | 进程只能运行在指定节点的CPU上 |
| `numactl --membind` | 绑定进程的内存分配策略 | ✅ **强制绑定** | 内存只能从指定节点分配，失败则进程无法运行 |
| `numa_run_on_node()` | 绑定进程到指定节点（CPU+内存） | ⚠️ **混合** | CPU绑定是硬亲和性，内存分配是偏好设置 |
| `numactl --preferred` | 设置内存分配偏好节点 | ⚠️ **偏好设置** | 优先从指定节点分配，但允许从其他节点分配 |
| `malloc()` / `calloc()` | 普通内存分配 | ⚠️ **受策略影响** | 在`numa_run_on_node()`后，会优先在绑定节点分配，但不保证 |
| `set_mempolicy()` | 设置内存分配策略 | 取决于策略类型 | `MPOL_BIND`强制，`MPOL_PREFERRED`偏好，`MPOL_INTERLEAVE`交错 |
| `numa_node_to_cpus()` | 获取节点的CPU列表 | ✅ **查询函数** | 仅查询，不设置任何策略 |
| `get_mempolicy()` | 获取内存策略 | ✅ **查询函数** | 仅查询，不设置任何策略 |

**关键区别说明：**

1. **强制保证 vs 偏好设置**：
   - **强制保证（硬亲和性）**：系统会严格遵循设置，如果无法满足（如内存不足），操作会失败
   - **偏好设置（软亲和性）**：系统会尽量满足设置，但在资源不足时允许从其他节点分配，不会失败

2. **`numa_run_on_node()`的特殊性**：
   - CPU绑定是**强制的（硬亲和性）**：进程只能运行在指定节点的CPU上
   - 内存分配是**偏好的**：优先在指定节点分配，但允许从其他节点分配
   - 这是为了平衡性能和可用性：如果绑定节点内存不足，进程仍能正常运行

3. **内存分配策略类型**：
   - `MPOL_BIND`（对应`--membind`）：强制绑定，内存必须从指定节点分配
   - `MPOL_PREFERRED`（对应`--preferred`）：偏好设置，优先从指定节点分配
   - `MPOL_INTERLEAVE`：交错分配，在多个节点间轮询分配
   - `MPOL_DEFAULT`：默认策略，由系统决定

**使用建议：**
- 需要**严格保证**内存位置：使用 `numa_alloc_onnode()` 或 `numactl --membind`
- 需要**高性能但允许灵活性**：使用 `numa_run_on_node()` 或 `numactl --preferred`
- 需要**精确控制CPU（硬亲和性）**：使用 `numactl --cpunodebind`、`numa_sched_setaffinity()` 或 `taskset`
- 需要**系统级强制CPU绑定（硬亲和性）**：使用 `cgroup` 的 `cpuset.cpus`

### 编程示例

#### 示例1：C语言中使用libnuma库

{% include_code perf/numa/numa_basic.c %}

编译命令：
```bash
gcc -o numa_example numa_basic.c -lnuma
```

#### 示例2：Python中使用numa库

{% include_code perf/numa/numa_python.py %}

#### 示例3：多线程程序中的NUMA优化

**方法一：不同线程绑定到不同NUMA节点**

当不同线程需要绑定到不同NUMA节点时，需要在线程内部手动设置CPU亲和性和内存分配：

{% include_code perf/numa/numa_threads_different_nodes.c %}

**方法二：所有线程绑定到同一NUMA节点**

`numa_run_on_node()` 是一个便捷函数，它会将当前进程绑定到指定NUMA节点的所有CPU上，并设置内存分配偏好策略为该节点。**注意**：CPU绑定是**硬亲和性**（强制的），但内存分配只是偏好设置（不强制保证）。

**使用场景：**
- 需要将整个进程（包括所有线程）绑定到特定NUMA节点
- 希望简化代码，避免手动管理CPU掩码和内存策略
- 在进程启动时进行NUMA绑定

如果所有线程都需要绑定到同一个NUMA节点，可以在主线程中使用 `numa_run_on_node()` 统一绑定：

{% include_code perf/numa/numa_threads_same_node.c %}

**numa_run_on_node的特点：**
- ✅ **优点**：代码简单，一个函数调用完成绑定
- ✅ **优点**：CPU绑定是**硬亲和性**（强制的），进程只能运行在指定节点的CPU上
- ⚠️ **注意**：内存分配是**偏好设置**，会优先在绑定的节点上分配，但不强制保证
- ⚠️ **限制**：所有线程都绑定到同一个节点，无法为不同线程分配不同节点
- ⚠️ **注意**：`numa_run_on_node()` 会影响整个进程及其所有线程

**选择建议：**
- 如果所有线程需要绑定到**同一个NUMA节点**：使用 `numa_run_on_node()`（方法二）
- 如果不同线程需要绑定到**不同的NUMA节点**：使用手动设置CPU亲和性（方法一）

编译命令：
```bash
gcc -o numa_threads_different_nodes numa_threads_different_nodes.c -lnuma -lpthread
gcc -o numa_threads_same_node numa_threads_same_node.c -lnuma -lpthread
```

### NUMA优化建议

1. **进程绑定**：将进程绑定到特定的NUMA节点，减少跨节点访问
2. **内存本地化**：在进程运行的节点上分配内存
3. **数据局部性**：确保数据访问模式与NUMA拓扑匹配
4. **监控工具**：使用 `numastat` 和 `numactl` 监控和调整NUMA策略
5. **应用程序设计**：在应用程序设计时考虑NUMA架构，合理分配线程和内存
6. **并行库适配**：注意并行库（如TBB、OpenMP）在初始化时会根据操作系统报告的硬件资源来决定线程池大小，通过限制可见的CPU资源可以间接控制并行库的线程数

## 参考资料

- `man numactl` - numactl命令手册
- `man numa` - NUMA库函数手册
- `/proc/sys/kernel/numa_balancing` - NUMA平衡配置
