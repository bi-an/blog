---
categories: cpp
date: 2024-01-30 11:07:39
tags:
- cpp
- basic
title: 计时工具
---

## times

1. bash built-in

```bash
times
```

2. function

```cpp
#include <sys/times.h>

clock_t times(struct tms *buf);
```

