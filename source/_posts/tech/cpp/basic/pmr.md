---
title: C++ PMR 详解
date: 2026-09-23 17:38:32
categories: cpp
tags:
 - cpp
 - basic
---

C++17 引入的 **PMR**（Polymorphic Memory Resource，多态内存资源）位于 `<memory_resource>`，核心类型包括 `std::pmr::memory_resource`、`std::pmr::polymorphic_allocator`，以及一组绑定了该分配器的容器别名（如 `std::pmr::vector`、`std::pmr::string`）。本文先剖析传统容器分配器模型的弊端，再说明 PMR 的统一性设计，以及内存池的上游回退机制。

## 传统容器的弊端

STL 容器将 `Allocator` 作为模板参数，在**编译期**固化分配策略：

```cpp
template<class T, class Allocator = std::allocator<T>>
class vector;
```

这在类型系统上精确，却带来三个工程症结。

**接口无法在类型层面统一。** `std::vector<T, A1>` 与 `std::vector<T, A2>` 是不同的类型。要在同一函数签名或同一容器中容纳“元素类型相同、分配策略不同”的对象，只能二选一：

- **函数写成模板**——把 `Allocator` 留作模板参数，编译器为每种分配器各生成一份函数：

  ```cpp
  template<class Alloc>
  void print_size(const std::vector<int, Alloc>& v) {
      std::cout << v.size() << '\n';
  }

  struct PoolAlloc { /* ... 满足 Allocator 要求 ... */ };

  std::vector<int>            a;
  std::vector<int, PoolAlloc> b;
  print_size(a);  // 实例化 print_size<std::allocator<int>>
  print_size(b);  // 实例化 print_size<PoolAlloc>
  ```

  代价：接口必须进头文件；模板函数**不能**是虚函数，进不了抽象基类的多态接口。

- **类型擦除包装**——自己造一层非模板外壳，用虚函数把具体 `vector<T, Alloc>` 藏起来。所谓类型擦除，就是抹掉编译期可见的 `Alloc` 差异，只保留运行期可调用的公共行为：

  ```cpp
  class AnyIntVector {
      struct Concept {
          virtual ~Concept() = default;
          virtual std::size_t size() const = 0;
      };
      template<class Alloc>
      struct Model : Concept {
          std::vector<int, Alloc> v;
          explicit Model(std::vector<int, Alloc> x) : v(std::move(x)) {}
          std::size_t size() const override { return v.size(); }
      };
      std::unique_ptr<Concept> self_;
  public:
      template<class Alloc>
      AnyIntVector(std::vector<int, Alloc> v)
          : self_(std::make_unique<Model<Alloc>>(std::move(v))) {}
      std::size_t size() const { return self_->size(); }
  };

  void print_size(const AnyIntVector& v) { std::cout << v.size() << '\n'; }

  std::vector<AnyIntVector> bag;
  bag.emplace_back(std::vector<int>{1, 2});
  bag.emplace_back(std::vector<int, PoolAlloc>{3, 4});
  ```

  代价：每个操作都要在 `Concept` 里声明；多一层虚调用与堆上 `Model`；为每个容器手写包装工程量大。标准库中的 `std::function`、`std::any` 也是同类手法。

**与虚函数机制冲突。** 虚函数在运行期决议类型，模板参数必须在编译期确定，二者无法直接组合：不能声明“元素类型固定、分配器可变”的虚接口再让派生类各自绑定不同 `Allocator`。

**嵌套结构中分配策略难以贯通。** 外层容器与内层元素各自持有默认分配器时，一次深拷贝会触发大量彼此无关的堆分配，无法共享缓存池，也难以统一生命周期与局部性。默认分配器下的典型情形：

```cpp
std::vector<std::string> vec1 = /* ... */;
std::vector<std::string> vec2 = vec1;  // 深拷贝：每个 string 各走一次堆分配
```

## PMR 的统一性设计

PMR 将“如何分配”从编译期模板参数剥离，下沉为运行期可替换的 `memory_resource`，再由类型擦除后的 `polymorphic_allocator` 统一挂接（对 `memory_resource*` 做类型擦除，标准库已实现）。容器类型因此保持一致——`std::pmr::vector`、`std::pmr::string` 等别名固定，变化的是构造时绑定的 `memory_resource*`——分配策略在运行期注入。于是非模板函数与虚接口可直接传递同一族容器，多级嵌套也可共享同一资源，分别对应上一节的三个症结。

以嵌套深拷贝为例，外层与内层绑定同一预分配资源：

```cpp
std::pmr::monotonic_buffer_resource pool{/* 预分配缓冲区 */};
std::pmr::vector<std::pmr::string> vec1{&pool};
std::pmr::vector<std::pmr::string> vec2{vec1, &pool};  // 深拷贝，仍走同一 resource
```

二者均通过同一套 `polymorphic_allocator` 向 `pool` 取内存。若底层是单调缓冲等预分配实现，后续请求往往只需在缓冲区内推进指针，而不必对每个字符串再走全局堆。“一次深拷贝”从 N 次散落的 `malloc` 收敛为对同一资源的连续操作。

| 维度 | 传统 `Allocator` 模板参数 | PMR |
|------|---------------------------|-----|
| 策略绑定时机 | 编译期，刻入类型 | 运行期，经指针注入 |
| 接口统一 | 不同分配器 → 不同类型 | 容器类型可保持一致 |
| 与虚函数 | 难以直接组合 | 可在多态边界传递同一族类型 |
| 嵌套深拷贝 | 易退化为多次独立堆分配 | 可共享同一资源，降低分配开销 |

## 内存池的上游回退机制

上一节案例依赖有限预分配缓冲。缓冲用尽时，程序不会无故崩溃；行为由该资源绑定的 **upstream（上游）** 决定。`monotonic_buffer_resource` 等标准池化实现普遍支持这种链式退路。

| 配置 | Buffer 耗尽后 | 适用场景 |
|------|---------------|----------|
| 默认（未指定 upstream） | 向 `get_default_resource()` 续借，初始即为 `new_delete_resource()` | 通用：本地加速，超出后平滑降级 |
| `null_memory_resource()` | 抛出 `std::bad_alloc`，绝不碰堆 | 硬实时 / 嵌入式 / 禁止堆分配 |
| 自定义 upstream | 转向指定的二级资源 | 多级 / 异构内存架构 |

**默认续借。** 只传缓冲、不指定 upstream 时，未满则在 buffer 内指针推进；已满则向上游申请更大的新块再划出本次所需。程序继续正确运行，超出部分退化为普通堆分配速度。

```cpp
std::array<std::byte, 1024> buffer;
std::pmr::monotonic_buffer_resource pool(buffer.data(), buffer.size());
std::pmr::vector<std::pmr::string> vec(&pool);
```

**禁止堆分配。** 将 upstream 设为 `null_memory_resource()`，超限即抛异常，不会私自向系统申请堆内存：

```cpp
std::pmr::monotonic_buffer_resource pool(
    buffer.data(), buffer.size(),
    std::pmr::null_memory_resource());

std::pmr::vector<int> vec(&pool);
try {
    for (int i = 0; i < 10000; ++i)
        vec.push_back(i);
} catch (const std::bad_alloc&) {
    // 预分配已耗尽
}
```

**增长策略。** 上游为默认堆时，并不是每缺一字节就 `malloc` 一次，而是几何扩容：先耗尽给定 Buffer（如 1KB），再向上游要约 2KB、4KB……把对 `::operator new` 的调用压到对数级。

**实现原理。** 本质是基类多态与组合式链式代理：`memory_resource` 为抽象基类；具体池既继承它（“我是资源”），又持有另一个 `memory_resource*`（“后备资源”）。基类采用 NVI——对外 `allocate`，对内纯虚 `do_allocate`：

```cpp
namespace std::pmr {
class memory_resource {
public:
    virtual ~memory_resource() = default;
    void* allocate(std::size_t bytes,
                   std::size_t alignment = alignof(std::max_align_t)) {
        return do_allocate(bytes, alignment);
    }
private:
    virtual void* do_allocate(std::size_t bytes, std::size_t alignment) = 0;
    virtual void  do_deallocate(void* p, std::size_t bytes, std::size_t alignment) = 0;
};
}
```

`monotonic_buffer_resource` 示意（非标准库原文）：

```cpp
class monotonic_buffer_resource : public memory_resource {
    void* current_ptr_;
    std::size_t remaining_;
    memory_resource* upstream_;  // 默认 get_default_resource()

public:
    monotonic_buffer_resource(void* buffer, std::size_t size,
                              memory_resource* upstream = get_default_resource())
        : current_ptr_(buffer), remaining_(size), upstream_(upstream) {}

protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        if (/* 当前块装得下 */)
            return /* 对齐并推进游标 */;

        std::size_t next = /* calculate_next_block_size(bytes) */;
        void* new_block = upstream_->allocate(next, alignment);
        current_ptr_ = new_block;
        remaining_   = next;
        return /* 从新块划出本次所需 */;
    }
};
```

调用链：

```text
vec.push_back()
  → polymorphic_allocator::allocate()
    → pool->allocate()
      ├─ Buffer 够用 → 指针偏移返回
      └─ 不够 → upstream_->allocate(下一块)
            ├─ new_delete_resource  → ::operator new
            ├─ null_memory_resource → throw std::bad_alloc
            └─ 其它自定义资源      → 其 do_allocate
```

池无需知晓上游具体类型，只依赖虚接口。因此可将 Pool A 的 upstream 设为 Pool B、再把 B 接到堆上，自然嵌套出多级分配器。
