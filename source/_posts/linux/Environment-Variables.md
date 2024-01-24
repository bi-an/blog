---
title: Environment Variables
date: 2024-01-23 21:49:40
categories: c/cpp
tags: syntax
---

## 环境变量

* `environ` - 用户环境，一个全局变量。头文件 `<unistd.h>`。可以通过 `man environ` 查看手册。

    ```cpp
    #include <unistd.h>
    #include <iostream>
    using namespace std;

    int main() {
        cout << "program environment: " << endl;
        for (char** entry = environ; *entry; ++entry) {
            cout << *entry << endl;
        }
    }
    ```

* `getenv` - 获取环境变量。头文件 `<stdlib.h>`。
* `setenv` - 设置环境变量。头文件 `stdlib.h`。
