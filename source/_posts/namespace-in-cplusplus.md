---
title: namespace in c++
tags: cpp
categories: Programming
date: 2023-11-02 09:53:52
---


## inline namespace

You can define and specialize members of an `inline namespace` as if they were also members of the enclosing namespace.
`inline namespace` 表示该命名空间也是上级命名空间的成员。
参考: [Inline namespace definitions (C++11)](https://www.ibm.com/docs/en/zos/2.2.0?topic=only-inline-namespace-definitions-c11)

例如：

```cpp
namespace A {
#if USE_INLINE_B
   inline
#endif
   namespace B {
      int foo(bool) { return 1; }
   }
   int foo(int) { return 2; }
}

int main(void) {
   return A::foo(true); // 如果USE_INLINE_B为0，则该句出错：找不到定义。
}
```

## Unnamed / anonymous namespaces vs static in namespace

Refer to: [Unnamed / anonymous namespaces vs static in namespace](https://medium.com/pranayaggarwal25/unnamed-namespaces-static-f1498741c527)

