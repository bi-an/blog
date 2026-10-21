---
title: ELF 符号检索方法
date: 2026-10-21 17:01:00
categories: tech
tags:
 - cpp
 - compile-link
 - linux
 - debug
---

在 Linux/Unix 环境下，分析 **ELF 格式文件**（包含目标文件 `.o`、可执行文件 `Executable` 以及共享库 `.so`）中的符号（Symbol）是排查依赖冲突、定位“未定义符号（Undefined Symbol）”报错以及进行逆向分析的核心手段。

由于编译产物（`.o`）与动态链接产物（`.so` / 目标程序）在符号的处理机制上存在差异，本文系统介绍查看 ELF 符号的工具、符号类型含义，以及运行时符号来源的检索方法。

<!-- more -->

## 一、查看 ELF 符号的常用工具与区别

Linux 提供了多种解析 ELF 结构的工具，最常用的包括 `nm`、`readelf`、`objdump` 和 `gdb`。

### 1. 核心工具详解

- **`nm` (Names)**：最快捷、最专注的符号查看器。
    - `nm -D file.so`：查看**动态符号表**（`.dynsym`），即运行时动态链接所需使用的符号。
    - `nm file`：查看**静态符号表**（`.symtab`，包含未 strip 的本地函数、全局变量等）。
    - 特点：输出精简、速度快，是 Bash 脚本批量处理和日常检索的首选。
- **`readelf` (Read ELF)**：最底层的 ELF 结构解析器。
    - `readelf -s file`：查看符号表。
    - `readelf -s -D file`：查看动态符号表。
    - 特点：不依赖 BFD 库，直接读取 ELF 文件头与节区，能精确获取符号的大小（Size）、绑定属性（Bind）、可见性（Vis）等底层数据。
- **`objdump` (Object Dump)**：兼顾反汇编与符号查看。
    - `objdump -t file`：查看静态符号表。
    - `objdump -T file`：查看动态符号表。
    - 特点：常用于配合反汇编代码（`-d`）或 Section 节区结构共同分析。
- **`gdb` (GNU Debugger)**：运行时的动态符号解析器。
    - 在 GDB 中使用 `print &symbol` 或 `info symbol <address>`。
    - 特点：属于运行时分析工具，能在程序载入内存并完成 GOT/PLT 重定位与 ASLR 地址偏移后，精准定位符号的运行时内存映射。

### 2. 工具对比一览

| 命令 | 主要用途 | 解析原理 | 最佳应用场景 |
| :--- | :--- | :--- | :--- |
| `nm` | 纯粹查看符号列表 | 依赖 BFD | 日常快速检索、写脚本批量查找 |
| `readelf` | 深度解析 ELF 内部结构 | 直读 ELF 头部（不依赖 BFD） | 查看符号具体大小、绑定属性，分析复杂/损坏的 ELF 文件 |
| `objdump` | 反汇编 + 符号查看 | 依赖 BFD | 需要结合汇编代码或 Section 结构分析符号 |
| `gdb` | 动态调试与运行时解析 | 运行时进程内存/符号重定向 | 排查多库同名符号冲突、动态加载（`dlopen`）及内存地址映射 |

### 3. 常见符号类型（Symbol Types）含义

使用 `nm` 或 `objdump` 时，输出结果中会有一个字母表示符号类型（**大写**表示全局/导出，**小写**表示局部/Local）：

- `T` / `t`：Text 段符号，通常是代码或函数。
- `D` / `d`：Data 段符号，已初始化的全局变量或静态变量。
- `B` / `b`：BSS 段符号，未初始化的全局变量。
- `U`：Undefined，未定义符号（当前 ELF 文件引用了该符号，但实体需要外部依赖在链接期或运行期提供）。

## 二、`.o` 与 `.so` 文件中符号的核心区别

虽然两者同属于 ELF 格式，但 `.o`（目标文件）与 `.so`（共享库）在符号的形态和作用上有本质不同：

1. **符号解析状态不同**：
    - `.o` 处于“编译期半成品”状态，包含大量待修正的未定义符号（`U`）和重定位表（Relocation Entries）。
    - `.so` 处于“链接完成”状态，内部符号大多已完成符号收集，仅保留动态运行期依赖外部的符号。
2. **符号表存在形式不同**：
    - `.o` 主要依赖**静态符号表（`.symtab`）**。
    - `.so` 必须包含**动态符号表（`.dynsym`）**；即使被 `strip` 剥离了 `.symtab`，只要保留 `.dynsym` 即可保证正常链接和运行。
3. **符号导出控制不同**：
    - `.o` 会暴露出源码中所有的非 `static` 全局符号。
    - `.so` 可以通过编译器属性（如 `visibility`）或 Linker Script 控制导出的符号，将辅助符号隐藏。

## 三、如何找到可执行文件中符号来自哪个 `.so`？

在动态链接的可执行文件里，未定义符号（类型为 `U`）会在程序运行时由动态链接器（`ld.so`）从各个共享库中查找并加载。查找符号归属有以下三种常用方式。

### 方法一：Shell 脚本遍历依赖库（最常用、最高效）

通过 `ldd` 列出可执行文件依赖的所有共享库，再结合 `nm` 遍历匹配目标动态符号：

```bash
for lib in $(ldd ./your_executable | awk '{print $3}' | grep '/'); do
    if nm -D "$lib" 2>/dev/null | grep -qw "your_symbol"; then
        echo "Found in: $lib"
    fi
done
```

### 方法二：使用 GDB 运行时解析

如果程序可以运行，利用 GDB 可以在运行时精准找出符号归属：

1. 启动 GDB 调试可执行文件：

    ```bash
    gdb -q ./your_executable
    ```

2. 在 GDB 内获取符号的地址：

    ```gdb
    (gdb) print &your_symbol
    ```

    输出示例：`$1 = (int (*)()) 0x7ffff789a123 <your_symbol@plt>`

3. 根据内存地址查看该地址所属的动态库文件：

    ```gdb
    (gdb) info symbol 0x7ffff789a123
    ```

    GDB 会直接输出该符号所在的动态库和偏移位置。

### 方法三：在指定系统目录中全局搜索

如果怀疑某个符号存在于系统库中，但不知道具体的 `.so` 文件名，可以在系统库目录（如 `/lib`、`/usr/lib`、`/usr/local/lib`）中全量搜索：

```bash
find /usr/lib /lib -name "*.so*" 2>/dev/null | xargs -I {} sh -c 'nm -D "{}" 2>/dev/null | grep -qw "your_symbol" && echo "Match: {}"'
```
