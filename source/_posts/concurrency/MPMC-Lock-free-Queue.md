---
title: 多生产者 - 多消费者 无锁队列
date: 2025-09-14 15:05:08
categories: concurrency
tags:
---

# 前言

这是阅读 Cameron Desrochers 的 [A Fast General Purpose Lock-Free Queue for C++](https://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++) 源码的笔记。

[Detailed Design of a Lock-Free Queue](https://moodycamel.com/blog/2014/detailed-design-of-a-lock-free-queue)

## 系统概览

MPMC 队列由一系列 SPMC 队列组成。
消费者使用启发式 (heuristic) 来决定消费哪个 SPMC 队列。
允许批量入列和出列，只需要很小的额外开销。

producer 需要一些 thread-local 数据；
consumer 也可以用一些可选的 thread-local 数据来加速；
这些 thread-local 数据可以与用户分配的 tokens 关联；
如果用户没有为生产者提供 tokens ，则使用无锁哈希表（以当前线程 ID 为键）来查找线程本地生产者队列：
每个 SPMC 队列都使用一个预分配的 token （或隐式分配的 token，如果没有提供的话）来创建。
由于 token 包含相当于线程特定的数据，因此它们不应该同时在多个线程中使用（尽管可以将 token 的所有权转移给另一个线程；特别是，这允许在线程池任务中使用令牌，即使运行任务的线程在中途发生变化）。

所有生产者队列都以无锁链表的形式连接在一起。当显式生产者不再有元素被添加时（即其令牌被销毁），它会被标记为与任何生产者都无关联，但它会保留在链表中，且其内存不会被释放；下一个新生产者会重用旧生产者的内存（这样，无锁生产者列表就只能添加）。隐式生产者永远不会被销毁（直到高层队列本身被销毁），因为无法知道给定线程是否已完成对数据结构的使用。需要注意的是，最坏情况下的出队速度取决于生产者队列的数量，即使它们都为空。

显式生产者队列和隐式生产者队列的生命周期存在根本区别：显式生产者队列的生产生命周期有限，与令牌的生命周期绑定；而隐式生产者队列的生产生命周期不受限制，且与高级队列本身的生命周期相同。因此，为了最大化速度和内存利用率，我们使用了两种略有不同的 SPMC 算法。通常，显式生产者队列设计得更快，占用的内存也更多；而隐式生产者队列设计得更慢，但会将更多内存回收到高级队列的全局池中。为了获得最佳速度，请始终使用显式令牌（除非您觉得它太不方便）。

任何分配的内存只有在高级队列被销毁时才会释放（尽管存在一些重用机制）。内存分配可以预先完成，如果内存不足，操作就会失败（而不是分配更多内存）。如果需要，用户可以覆盖各种默认大小参数（以及队列使用的内存分配函数）。

## Full API (pseudocode)

```bash
# Allocates more memory if necessary
enqueue(item) : bool
enqueue(prod_token, item) : bool
enqueue_bulk(item_first, count) : bool
enqueue_bulk(prod_token, item_first, count) : bool

# Fails if not enough memory to enqueue
try_enqueue(item) : bool
try_enqueue(prod_token, item) : bool
try_enqueue_bulk(item_first, count) : bool
try_enqueue_bulk(prod_token, item_first, count) : bool

# Attempts to dequeue from the queue (never allocates)
try_dequeue(item&) : bool
try_dequeue(cons_token, item&) : bool
try_dequeue_bulk(item_first, max) : size_t
try_dequeue_bulk(cons_token, item_first, max) : size_t

# If you happen to know which producer you want to dequeue from
try_dequeue_from_producer(prod_token, item&) : bool
try_dequeue_bulk_from_producer(prod_token, item_first, max) : size_t

# A not-necessarily-accurate count of the total number of elements
size_approx() : size_t
```

## Producer Queue (SPMC) Design

### 隐式和显式版本的共享设计

生产者队列由块组成（显式和隐式生产者队列使用相同的块对象，以实现更好的内存共享）。初始状态下，它没有块。每个块可以容纳固定数量的元素（所有块的容量相同，均为 2 的幂）。此外，块包含一个标志，指示已填充的槽位是否已被完全消耗（显式版本使用此标志来判断块何时为空），以及一个原子计数器，用于计数已完全出队的元素数量（隐式版本使用此标志来判断块何时为空）。

为了实现无锁操作，生产者队列可以被认为是一个抽象的无限数组。尾部索引指示生产者下一个可用的槽位；它同时也是已入队元素数量的两倍（**入队计数 (enqueue count)**）。尾部索引仅由生产者写入，并且始终递增（除非溢出并回绕，但就我们的目的而言，这种情况仍被视为“递增”）。由于只有一个线程在更新相关变量，因此生产一个元素的过程非常简单。头索引指示下一个可以被消费的元素。头索引由消费者原子地递增，可能并发进行。为了防止头索引达到/超过感知到的尾部索引，我们使用了一个额外的原子计数器：**出队计数 (dequeue count)**。出队计数是乐观的，即当消费者推测有元素需要出队时，它会递增。如果出队计数在递增后的值小于入队计数（尾部），则保证至少有一个元素要出队（即使考虑到并发性），并且可以安全地递增头部索引，因为知道之后它会小于尾部索引。另一方面，如果出队计数在递增后超过（或等于）尾部，则出队操作失败，并且出队计数在逻辑上会递减（以使其最终与入队计数保持一致）：这可以通过直接递减出队计数来实现，但是（为了增加并行性并使所有相关变量单调递增），改为递增**出队过量提交计数器 (dequeue overcommit counter)**。

```
出队计数的逻辑值 = 出队计数变量 - 出队过量提交值
```

在消费时，一旦如上所述确定了有效索引，仍然需要将其映射到一个块以及该块中的偏移量；为此会使用某种索引数据结构（具体使用哪种结构取决于它是隐式队列还是显式队列）。最后，可以将元素移出，并更新某种状态，以便最终知道该块何时完全消费。下文将分别在隐式和显式队列的各个部分中对这些机制进行完整描述。

如前所述，尾部和头部的索引/计数最终会溢出。这是预料之中的，并且已被考虑在内。因此，索引/计数被视为存在于一个与最大整数值大小相同的圆上（类似于 360 度的圆，其中 359 在 1 之前）。为了检查一个索引/计数（例如 a）是否位于另一个索引/计数（例如 b）之前（即逻辑小于），我们必须确定 a 是否通过圆上的顺时针圆弧更接近 b。使用以下循环小于算法（32 位版本）：a < b 变为 a - b > (1U << 31U)。a <= b 变为 a - b - 1ULL > (1ULL << 31ULL)。请注意，循环减法“仅适用于”普通无符号整数（假设为二进制补码）。需要注意的是，尾部索引的增量不会超过头部索引（这会破坏队列）。请注意，尽管如此，从技术上讲仍然存在竞争条件，即消费者（或生产者）看到的索引值过于陈旧，几乎比当前值落后一整圈（甚至更多！），从而导致队列的内部状态损坏。但在实践中，这不是问题，因为遍历 2^31 个值（对于 32 位索引类型）需要一段时间，而其他核心到那时会看到更新的值。实际上，许多无锁算法都基于相关的标签指针习语，其中前 16 位用于重复递增的标签，后 16 位用于指针值；这依赖于类似的假设，即一个核心不能将标签递增超过 2^15 次，而其他核心却不知道。尽管如此，队列的默认索引类型是 64 位宽（如果 16 位看起来就足够了，那么理论上应该可以避免任何潜在的竞争）。

