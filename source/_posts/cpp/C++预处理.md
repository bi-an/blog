---
title: C++ 预处理
date: 2024-01-07 16:40:10
categories: c/cpp
tags: syntax
---

## 预定义宏

`#if defined(__linux)`
`#ifdef LINUX2`

## C标准预定义宏

* `__LINE__`
* `__func__`
* `__FILE__`
* `NDEBUG`：参考[_DEBUG和NDEBUG的区别](https://stackoverflow.com/questions/2290509/debug-vs-ndebug)，其中，`_DEBUG`是Visual Studio定义的，`NDEBUG`是C/C++标准。

## GNU C预定义宏

[官方文档](https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html)

* `__COUNTER__`: 扩展为从`0`开始的连续整数值，每次在源码中出现，则加`1`。不同源文件的`__COUNTER__`互不影响。

  可以用来生成唯一的命名。
  参考[链接](https://stackoverflow.com/questions/652815/has-anyone-ever-had-a-use-for-the-counter-pre-processor-macro)。

  ```cpp
  #define CONCAT_IMPL(x,y) x##y
  #define CONCAT(x,y) CONCAT_IMPL(x,y)
  #define VAR(name) CONCAT(name,__COUNTER__)
  int main() {
      int VAR(myvar); // 展开为 int myvar0;
      int VAR(myvar); // 展开为 int myvar1;
      int VAR(myvar); // 展开为 int myvar2;
  }
  ```

* `program_invocation_name`：参考[man page](https://man7.org/linux/man-pages/man3/program_invocation_name.3.html)
* 


## #pragma

### #pragma weak

#### Synopsis

```cpp
#pragma weak function-name1 [= function-name2]
```

`#pragma weak` means that even if the definition of the symbol is not found, no error will be reported.

#### Example

```cpp
#include <stdio.h>

// It is not an error for symbol to never be defined at all.
// Without this line, the address of "foo" will always evaluate to "true",
// so the linker will report an "undefined reference to 'foo'" error.
#pragma weak foo
// The declaration is needed.
/* extern */ void foo();

int main() {
    if (foo)
        foo();

    return 0;
}
```

Reference: [1](https://alberand.com/weak-directive.html)、[2](https://gcc.gnu.org/onlinedocs/gcc-4.5.4/gcc/Weak-Pragmas.html)、[3](https://docs.oracle.com/cd/E19059-01/wrkshp50/805-4955/auto6/index.html)

