---
title: 汇编与 gdb
date: 2025-08-22 15:42:27
categories: c/cpp
tags:
---

## 前言

我们在调试 release 版本的程序时，由于缺乏符号信息，所以需要通过寄存器来查看函数的参数、返回值等。

## 寄存器

x86_64 架构常用寄存器

|   寄存器  |               用途说明              |
|:---------:|:-----------------------------------:|
| rip       | 当前指令地址（程序计数器）          |
| rsp       | 栈顶指针                            |
| rbp       | 栈帧基址                            |
| rax       | 函数返回值                          |
| rdi~r9    | 函数参数（前六个整数 / 指针类型参数） |
| r10~r15   | 临时寄存器或扩展参数                |
| xmm0~xmm7 | 浮点参数（如 double, float）        |

其中，函数参数寄存器：

| 寄存器 |  参数位置  |                      用途说明                      |
|:------:|:----------:|:--------------------------------------------------:|
| rdi    | 第一个参数 | 通常用于传递指针或整数，如字符串地址、结构体指针等 |
| rsi    | 第二个参数 | 常用于传递长度、标志位、或第二个数据指针           |
| rdx    | 第三个参数 | 可用于传递大小、偏移量、或第三个数据值             |
| rcx    | 第四个参数 | 常用于计数器、索引或第四个参数                     |
| r8     | 第五个参数 | 扩展参数，用于传递更多数据或标志位                 |
| r9     | 第六个参数 | 同样用于扩展参数，如结构体成员或函数指针           |

## gdb 查看寄存器命令

```bash
# 查看所有寄存器
info registers

# 查看指定寄存器
info registers rdi
# 或简写
i r rdi

# 查看十六进制
p/x $rdx    # 十六进制
p/d $rdx    # 十进制
# 或简写
p $rdx
```

其中，`info registers` 会打印三列：
- 第一列：寄存器名称
- 第二列：寄存器的值（十六进制）
- 第三列：寄存器的值（十进制；也可能是十六进制，用 `0x` 开头）

`info registers rdi` 与 `p $rdi` 效果相同。

从寄存器查到的内存地址，可以用 `x` （examinze）命令来查看内存的值：

```bash
# 查看指令
x/i $rip
# 查看栈顶
x/16x $rsp

# 查看内存
p $rdi
x/2gx $rdi
# 或先用 $rdi 查出内存地址，直接用地址访问
x/2gx 47926411878160
```

`x` 命令的说明：

```bash
x/FMT ADDRESS
```

其中：

- `x`：表示“examine memory”（查看内存）
- `2`：数字，表示要查看的单元数
- `g`：表示每个单元的 size ，有 b(byte), h(halfword), w(word), g(giant, 8 bytes)
- `x`：表示值的格式，有 o(octal), x(hex), d(decimal), u(unsigned decimal), t(binary), f(float), a(address), i(instruction), c(char), s(string)
  and z(hex, zero padded on the left).

在 gdb 命令行中使用 `help` 命令，可以查看命令的说明。

```bash
(gdb) help x
Examine memory: x/FMT ADDRESS.
ADDRESS is an expression for the memory address to examine.
FMT is a repeat count followed by a format letter and a size letter.
Format letters are o(octal), x(hex), d(decimal), u(unsigned decimal),
  t(binary), f(float), a(address), i(instruction), c(char), s(string)
  and z(hex, zero padded on the left).
Size letters are b(byte), h(halfword), w(word), g(giant, 8 bytes).
The specified number of objects of the specified size are printed
according to the format.  If a negative number is specified, memory is
examined backward from the address.

Defaults for format and size letters are those previously used.
Default count is 1.  Default address is following last thing printed
with this command or "print".
```

## `frame` 与寄存器的值

- GDB 中的寄存器值（如 `$rax`, `$rdi`, `$rsp` 等）是当前 CPU 执行上下文的快照。
- 当你切换到 `frame 0`（最内层栈帧） 时，寄存器值是最真实的，因为这是程序当前正在执行的地方。
- 当你切换到 外层栈帧（frame 1, 2, ...） 时，GDB 会尝试还原当时的寄存器状态，但这依赖于：
  - 编译器是否保存了寄存器值（如 callee-saved）
  - 是否有调试符号或 unwind 信息
  - GDB 是否能推断出寄存器的保存位置

寄存器值可能出现的情况

|                      情况                     |                  表现                  |
|:---------------------------------------------:|:--------------------------------------:|
| 寄存器是 caller-saved（如 rdi, rsi, rax）     | 可能显示 <not saved> 或错误值          |
| 寄存器是 callee-saved（如 rbx, rbp, r12~r15） | 通常能正确还原                         |
| 没有调试信息或优化严重                        | GDB 无法还原，显示当前值或 <not saved> |

建议

- 如果你要分析寄存器状态，最好在 frame 0 或断点处进行。
- 如果你在分析 core dump 或栈破坏问题，寄存器值只能作为参考，不要完全依赖外层 frame 的寄存器快照。
- 使用 info args 和 info locals 更可靠地查看参数和局部变量（如果有符号信息）。
