---
title: 原子操作
tags:
---


## 定义

在该值被读的瞬间和被替换的瞬间之间，其他线程不能修改该值。

the value cannot be modified by other threads between the instant its value is read and the moment it is replaced. - Refer to [std::atomic::compare_exchange_weak](https://cplusplus.com/reference/atomic/atomic/compare_exchange_weak/)

## 比较交换（CAS）

``` cpp
(1) bool compare_exchange_weak (T& expected, T val, memory_order sync = memory_order_seq_cst) volatile noexcept;bool compare_exchange_weak (T& expected, T val, memory_order sync = memory_order_seq_cst) noexcept;

(2) bool compare_exchange_weak (T& expected, T val, memory_order success, memory_order failure) volatile noexcept;bool compare_exchange_weak (T& expected, T val, memory_order success, memory_order failure) noexcept;
```

    如果expected和内存值相同，则用val替换内存值，并返回true；
    如果不同，则用内存值替换expected，并返回false。
    整个CAS过程是原子的。

“用val替换内存值”：这是我们的最终目的，将内存值设置为val。因为expected和内存值相同，认定内存没有被其他线程修改过，见下句。

“用内存值替换expected”：在下一次比较（可能经历过线程切换）时，如果expected与内存相同，则认为内存没有被其他线程修改过（其实这种认定不一定正确，参见**ABA问题**），于是放心地将内存值设置为val。

### 虚假失败（fail spuriously）

    即使expected和内存值相同，还是可能返回false。

因为这种情况本应是比对成功的，所以称此次比对为“虚假失败”。虚假失败时，会返回false，但是没有修改expected。

虚假失败不一定成功将内存值设置为val，所以需要下一次while循环重新尝试。

weak版本的CAS由于没有严格不允许虚假失败，所以在某些平台上可能效率比strong版本的CAS更高一些。参见[更多细节](https://stackoverflow.com/questions/355365/what-does-spurious-failure-on-atomicinteger-weakcompareandset-mean)。

## 内存序