---
title: 详解RCU机制
date: 2025-08-17 10:39:19
categories: 操作系统
tags: 线程
---

## RCU (读-复制-更新)

RCU（Read-Copy-Update，读-拷贝-更新）是 Linux 内核中一种高效的 并发读-写同步机制，专门用于在多核系统下实现 大量读、少量写 的场景。它的核心思想是：读操作完全无锁，写操作通过复制更新，最后再安全地回收旧版本。

RCU 的核心思想

- 读操作直接访问数据结构，不加锁
- 写操作：
  - 先 复制原数据 → 修改副本
  - 更新指针，使新数据生效
  - 延迟回收旧数据，保证当前正在读取旧数据的线程不受影响

### `__rcu`

它是 RCU 机制中非常关键的一环，用来让 编译器和内核知道某个指针是 RCU 保护的。

1. `__rcu` 的定义

在 Linux 内核中（以 x86_64 为例）：

```c
#define __rcu
```

实际上，它 本身对编译器不产生直接影响

主要用途是 标记类型，告诉 Sparse 静态分析工具 或 内核开发者：这个指针受 RCU 保护，不能随意直接读/写

也就是说，`__rcu` 是 一个注释性质的宏，编译器编译时忽略，但静态分析工具会检查 RCU 访问规则。

2. `__rcu` 的作用

标记 RCU 保护的指针，常见用法：

```c
struct files_struct {
    struct fdtable __rcu *fdt;
    struct file __rcu *fd_array[NR_OPEN_DEFAULT];
};
```

含义：

- `fdt` 是 RCU 指针
- 不能直接写 `fdt = new_ptr`; 或直接解引用 `fdt->xxx`
- 必须通过 RCU API，如：
  - `rcu_assign_pointer(fdt, new_fdt);` → 安全更新指针
  - `rcu_dereference(fdt)` → 安全读取指针
- 这样可以保证：
  - 写线程更新指针时不会破坏读线程的访问
  - 读线程可以无锁访问旧数据

3. `__rcu` 与编译器和内核

- 静态检查（Sparse）
  - Sparse 是 Linux 内核推荐的静态分析工具
  - 它会检查：
    - 所有 `__rcu` 指针的写操作是否用 `rcu_assign_pointer`
    - 所有读取是否用 `rcu_dereference`
  - 如果直接访问，就会报错，避免 RCU 访问错误
- 内存屏障和优化
  - `rcu_assign_pointer` 内部会加上 适当的写屏障 (`smp_wmb()`)
  - `rcu_dereference` 内部会加 读取屏障 (`smp_rmb()`)
  - 防止编译器/CPU 重排导致读写顺序错误

4. `__rcu` 的核心原理总结

| 方面    | 说明                                                     |
| ----- | ------------------------------------------------------ |
| 编译器作用 | 本身是空宏，不改变代码                                            |
| 静态分析  | Sparse 检查 RCU 指针的安全读写                                  |
| 内存屏障  | 通过 `rcu_assign_pointer` / `rcu_dereference` 添加屏障，保证并发安全 |
| 运行时   | 指针仍然是普通指针，实际存储和访问和普通指针一样                               |
