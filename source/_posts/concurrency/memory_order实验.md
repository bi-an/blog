---
title: memory_order测试
date: 2025-09-24 14:32:53
categories:
tags:
---

## relaxed

- 问：load(relaxed) 看到的是不是最新值？
- 答：是的。

经过多次测试（包括 TSAN），CAS(relaxed) 计数器都是正确的：

```bash
g++ -fsanitize=thread test_memory_order_relaxed.cpp -lpthread
```

{% include_code lang:cpp atomic/test_memory_order_relaxed.cpp %}
