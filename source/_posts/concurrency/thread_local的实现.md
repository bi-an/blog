---
title: thread_local的实现
date: 2025-08-19 19:41:37
categories: c/cpp
tags: 线程
---


1. thread_local 的基本语义

C++11 引入的存储类型说明符：

thread_local int x = 0;


表示 每个线程都有一份独立的 x，互不干扰。

生命周期：跟普通静态变量类似（全局存活直到线程退出）。

2. 底层实现原理

它的实现依赖于 TLS (Thread Local Storage, 线程局部存储) 机制。

在 ELF/Linux 下：

编译器在 .tdata / .tbss 段里为 thread_local 变量分配空间（就像全局变量在 .data / .bss 段里）。

程序加载时，动态链接器（ld.so）会记录这些 TLS 变量的“模板布局”。

每个线程启动时，线程库（glibc/pthread）会：

给这个线程分配一块 TLS 块（通常放在线程栈附近，或者专门的内存页）。

把 .tdata 里的初始值拷贝到这个线程的 TLS 块。

.tbss 部分（未初始化的 thread_local）则清零。

线程访问 thread_local 时，编译器生成的代码会通过 TLS 寄存器（如 x86-64 的 FS/GS 段寄存器）+ 偏移量，找到对应线程的存储单元。

例如 x86-64 Linux 上，errno 就是：

#define errno (*__errno_location())


而 __errno_location() 内部就是通过 %fs:offset 找到 TLS 块里的 errno。

3. 存放在哪里？

Windows：在 TEB (Thread Environment Block) 里有 TLS 指针，__declspec(thread) 就用它。

Linux/ELF：在每个线程的 TLS 块里（通常分配在线程栈附近的一片内存区域）。访问通过 FS/GS 寄存器。

编译器细节：

GCC/Clang 默认用 “动态 TLS 模型”（访问时通过动态链接器查询 TLS 偏移）。

如果加 -ftls-model=initial-exec，编译器会直接用固定偏移访问 TLS，速度更快（但要求变量在主程序或静态库里）。

4. 示例

```cpp
#include <iostream>
#include <thread>
thread_local int counter = 0;

void worker(int id) {
    for (int i = 0; i < 3; i++) {
        counter++;
        std::cout << "Thread " << id << ": counter = " << counter << "\n";
    }
}

int main() {
    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    t1.join();
    t2.join();
}
```

输出大致是：

```
Thread 1: counter = 1
Thread 1: counter = 2
Thread 1: counter = 3
Thread 2: counter = 1
Thread 2: counter = 2
Thread 2: counter = 3
```

说明 counter 在不同线程里独立。

✅ 总结：

thread_local 变量放在 TLS 段，每个线程有自己的拷贝。

访问是通过 线程局部存储寄存器（FS/GS）+ 偏移量 实现的。

存储空间由线程库在创建线程时分配和初始化。