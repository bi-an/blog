---
title: Print The Stack Trace of A Process
date: 2024-01-23 22:33:09
categories: c/cpp
tags: syntax
---

## 函数

[glic Functions](https://www.gnu.org/software/libc/manual/html_node/Backtraces.html)

    backtrace
    backtrace_symbols
    backtrace_symbols_fd

```cpp
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void print_trace() {
    static const int SIZE = 10;

    void* buffer[SIZE];
    char** strings;
    int size, i;

    size = backtrace(buffer, SIZE);
    strings = backtrace_symbols(buffer, size);
    if (strings != NULL) {
        printf("Obtained %d stack frames.\n", size);
        for (i = 0; i < size; ++i)
            printf("%s\n", strings[i]);
        free(strings);
    }
}
```

## 第三方库

breakpad: https://blog.csdn.net/weixin_45609638/article/details/125090204

https://juejin.cn/post/6899070041074073614

https://juejin.cn/post/7130820896213712927