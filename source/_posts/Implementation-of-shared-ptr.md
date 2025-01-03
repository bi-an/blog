---
title: shared_ptr 的实现
date: 2024-09-26 16:44:29
tags: c/cpp
---

## 参考源码

> /usr/include/c++/11/bits/shared_ptr_base.h
> /usr/include/c++/11/bits/shared_ptr.h

博客：https://zhiqiang.org/coding/std-shared-ptr.html

## `std::shared_ptr` 的性质

1. 复制构造、析构是线程安全的。

标准库使用原子操作实现无锁的线程安全性。

2. 写操作（例如 `reset` 、 赋值 `operator=` ）是线程不安全的。

写操作和复制构造、析构的主要区别是：

* 复制构造、析构函数中，单个线程只处理一个对象，复制构造函数将其他对象复制过来之后，不会改动其他对象的资源（引用计数、所管理的内存）。
* 但是写操作可能多个线程都在处理该 shared_ptr 。例如多个线程都对同一个 shared_ptr 进行赋值：

```cpp
shared_ptr<int> sp1 = make_shared<int>(1);
sp1 = sp2; // 线程 1
sp1 = sp3; // 线程 2
```

对比源码分析：

```cpp
      __shared_count&
      operator=(const __shared_count& __r) noexcept
      {
	_Sp_counted_base<_Lp>* __tmp = __r._M_pi;
	if (__tmp != _M_pi)
	  {
	    if (__tmp != nullptr)
	      __tmp->_M_add_ref_copy();
	    if (_M_pi != nullptr)
	      _M_pi->_M_release(); // 注意：线程 1 和线程 2 持有相同的 _M_pi！
	    _M_pi = __tmp; // 注意：线程 1 和线程 2 持有相同的 _M_pi！
	  }
	return *this;
      }
```

以上代码中两个“注意”可能同时在发生，例如：
线程 1 release 的时候，线程 2 在给 _M_pi 赋值；
或者两个线程同时在 release 或同时在给 _M_pi 赋值。

## 要点

1. 管理的内存和引用计数都应该动态分配到堆（ heap ）上，这样多个 shared_ptr 对象才能更新同一份数据。
2. 需要同时维护强引用计数和弱引用计数。
3. 引用计数本身应该是一个控制块类，使用 `delete this` 来自动删除（析构）引用计数。

永远不要手动 `delete use_count`，因为其他线程可能此时正在使用该资源，例如解引用 `*use_count`。

{% include_code SharedPtr.hpp lang:cpp from:1 SharedPtr.hpp %}

## 参考

https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_concurrency.html