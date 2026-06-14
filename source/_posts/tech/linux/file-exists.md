---
categories: linux
date: 2024-09-06 18:09:03
tags:
- linux
- shell
title: File Exists
---

# 判断文件是否存在的方法

## 相关函数

在 Linux 系统中，判断文件是否存在的常用函数包括：

- **`access`** - 检查文件的访问权限
- **`stat`** - 获取文件状态信息
- **`inotify_node`** - 监控文件系统事件
- **`opendir`** - 打开目录
- **`readdir`** - 读取目录内容

## 使用方法

### 方法一：Linux `access` 函数

`access` 函数用于检查调用进程是否可以对文件执行指定的操作。

```cpp
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    // amode 参数可以是以下值的组合：
    // R_OK - 检查读权限
    // W_OK - 检查写权限
    // X_OK - 检查执行权限
    // F_OK - 检查文件是否存在
    
    if (access("a.txt", R_OK) == -1) {
        perror("a.txt is not readable");
        exit(1);
    }

    printf("a.txt exists and is readable\n");
    return 0;
}
```

### 方法二：C++ `ifstream`

使用 C++ 标准库的文件流来检查文件是否存在。

```cpp
#include <fstream>
#include <iostream>
#include <cstdlib>

int main() {
    std::ifstream file("a.txt", std::ifstream::in);
    
    if (!file) {
        std::cerr << "a.txt doesn't exist or cannot be opened" << std::endl;
        exit(1);
    }
    
    std::cout << "a.txt exists and can be opened" << std::endl;
    file.close();
    
    return 0;
}
```

## 注意事项

### 文件系统缓存问题

> ⚠️ **重要提示**：文件系统并不会实时刷新缓存，尤其是在网络文件系统中。

这会导致以下现象：

- 文件即使已经创建，`access` 或 `stat` 函数依然可能返回 **"No such file"**
- `ls` 命令可以看到文件，但程序调用却失败
- 这是因为 `ls` 和 `access`/`stat` 函数的实现机制不同

### NFS 文件系统的典型问题

在网络文件系统（NFS）中，这个问题尤为明显：

1. **创建文件后立即检查**
   - 创建文件之后立即调用 `stat` 命令查看文件
   - `stat` 可能会报告 **"文件不存在"**

2. **删除文件后仍有残留**
   - 删除文件之后立即调用 `stat` 命令查看文件
   - `stat` 依然可能看到该文件（缓存未更新）

### 解决方案

对于需要实时感知文件系统变化的场景，可以改用 **`readdir`** 来读取目录：

- 目录的缓存更新速度通常比单个文件的属性更快
- 通过遍历目录内容来判断文件是否存在，可以获得更实时的结果

```cpp
#include <dirent.h>
#include <string.h>

bool file_exists_in_dir(const char* dir_path, const char* filename) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        return false;
    }
    
    struct dirent* entry;
    bool found = false;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, filename) == 0) {
            found = true;
            break;
        }
    }
    
    closedir(dir);
    return found;
}
```
