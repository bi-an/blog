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

- `x`：表示 “examine memory”（查看内存）
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

## 在特定线程中设置断点

### 断点只作用于某线程

```bash
# 查看所有线程 ID 和当前线程 ID（gdb 中会使用 * 标注当前线程）
(gdb) info threads
# 切换当前上下文到指定线程
(gdb) thread <THREAD_ID>
# 通过查看当前堆栈是不是自己要断点的线程
(gdb) bt
(gdb) break LOCATION thread THREADNUM
# 条件断点
(gdb) break source.c:123 thread 5 if fds[0].fd == 7
# 如果没有 debug 符号，可以利用函数返回值寄存器断点
(gdb) break poll thread 2 if $rdx > 0
# 完整格式
break [PROBE_MODIFIER] [LOCATION] [thread THREADNUM] [if CONDITION]
```

### 锁定调度器，只让当前线程运行

默认情况下，GDB 会让所有线程一起运行（比如你执行 continue 时）。如果你只想让当前线程运行，其它线程保持暂停，可以使用：

```bash
(gdb) set scheduler-locking on
```

这表示：只有当前线程会执行，其他线程全部暂停。

其中模式还有：

| 模式 |                          说明                         |
|:----:|:-----------------------------------------------------:|
| off  | 默认值，所有线程都可以运行                            |
| on   | 只有当前线程运行，其他线程暂停                        |
| step | 单步调试时只运行当前线程，continue 时其他线程也会运行 |

你可以随时切换：

```bash
(gdb) set scheduler-locking step
```

- 如果你在调试死锁、竞态或线程间通信问题，锁定调度器是非常有效的方式。
- 如果你在调试某个 poll() 或 epoll_wait() 调用，只想观察某个线程的行为，可以结合 catch syscall 和 thread 命令一起使用。


## 查看汇编代码

```bash
# 查看汇编代码，其中 "=>" 标记的是当前执行位置
(gdb) disassemble
# 反汇编指定地址范围
# 这会显示从当前指令开始的 32 字节范围内的汇编代码。
(gdb) disassemble $rip, $rip+32

# 查看当前指令（x86）
(gdb) x/i $pc
# 或在 x86-64 架构下：
(gdb) x/i $rip

# Bonus: 默认 GDB 使用 AT&T 风格（如 %rax），你可以切换为 Intel 风格
# 这样输出会更接近你在汇编教材或 IDA Pro 中看到的格式
(gdb) set disassembly-flavor intel
```

## 位置无关代码（PIC）

- 位置无关代码（`PIC`）：程序可以在内存中的任意位置运行，不需要修改代码中的绝对地址。
- 节省空间：相比使用 64 位绝对地址，RIP 相对寻址只需要一个 32 位偏移量。
- 更安全：支持地址随机化（ASLR），提高程序的安全性。

在 x86-64 架构中，传统的绝对地址寻址方式不再适用于位置无关代码。于是引入了 RIP（指令指针）相对寻址：

假设你有一个全局变量 int x = 42;，在汇编中访问它可能会变成：

```asm
asm
mov eax, DWORD PTR [rip + offset_to_x]
```

这里的 offset_to_x 是编译器计算出来的 x 相对于当前指令的偏移量。


|    寻址方式    |                 描述                | 是否位置无关 |
|:--------------:|:-----------------------------------:|:------------:|
| 绝对地址寻址   | 使用固定地址，如 [0x400123]         | ❌ 否         |
| 寄存器间接寻址 | 如 [rax]，地址由寄存器决定          | ✅ 是         |
| RIP 相对寻址   | 如 [rip + offset]，相对当前指令位置 | ✅ 是         |



但并不是所有 PIC 都用 RIP 相对寻址，PIC 的实现方式取决于：

- 架构：在 x86（32 位）中没有 RIP 寄存器，PIC 通常通过 call 指令获取当前地址，再加偏移量。
- 编译器策略：有些编译器会使用全局偏移表（GOT）或过程链接表（PLT）来实现位置无关性。
- 访问目标：访问函数地址时可能通过 PLT；访问外部变量时可能通过 GOT；访问静态数据时可能用 RIP 相对寻址。

|    架构    |   是否使用 RIP 相对寻址  |  是否支持位置无关代码  |
|:----------:|:------------------------:|:----------------------:|
| x86-64     | ✅ 常用，尤其访问数据段   | ✅ 强力支持（默认启用） |
| x86 (32位) | ❌ 无 RIP，用其他方式实现 | ✅ 但需要特殊技巧       |

举个 gdb 调试的例子：

```bash
(gdb) x/i $rip
=> 0x2ac084b5ec10 <poll>:       cmpl   $0x0,0x2d939d(%rip)        # 0x2ac084e37fb4 <__libc_multiple_threads>
(gdb) p (bool)$__libc_multiple_threads
true
```

- `cmpl $0x0, 0x2d939d(%rip)` 是一条比较指令（`cmp`），用于将某个内存地址中的值与立即数 `0` 进行比较。
- `(%rip)` 表示使用 RIP 相对寻址，这是 x86-64 架构中常见的一种寻址方式。
- 实际比较的是地址 `0x2ac084e37fb4` 处的值，也就是 `__libc_multiple_threads` 这个变量。

`__libc_multiple_threads` 是什么？

- 这是 GNU C 库（glibc）中的一个内部变量，用来标记当前进程是否启用了多线程。
- 如果这个值是 0，说明当前进程是单线程。
- 如果是非零，说明进程中有多个线程。

所以这条指令的作用是：判断当前进程是否是多线程环境，可能用于决定是否启用线程安全的行为。


### 为什么使用 RIP 相对寻址？

1. RIP 是唯一始终已知的寄存器
- 在执行指令时，CPU总是知道当前指令的地址（即 RIP）。
- 所以可以在编译时计算出目标数据与当前指令之间的偏移量，而不需要知道数据的绝对地址。

这就允许编译器生成位置无关代码，即使程序被加载到不同的内存地址，偏移量仍然有效。

2. 其他寄存器值是动态的，不可预测
- 比如 RBX、RAX、RDI 等寄存器，它们的值在运行时可能被程序修改。
- 如果用这些寄存器做基址寻址，编译器就无法提前知道它们的值，也就无法生成稳定的偏移量。

3. 支持共享库和地址空间布局随机化（ASLR）
- RIP 相对寻址让代码段不依赖固定地址，可以被多个进程共享。
- 也支持操作系统在运行时随机加载地址，提高安全性（ASLR）。

4. 节省指令空间
- 使用 RIP 相对寻址只需要一个 32 位偏移量。
- 如果使用绝对地址，需要嵌入完整的 64 位地址，指令长度更长，效率更低。

### 为什么使用 RIP 相对寻址只需要一个 32 位偏移量

在 x86-64 架构中，RIP 相对寻址的偏移量被设计为一个有符号的 32 位整数，也就是一个 displacement（位移）字段，它在机器码中只占用 4 个字节。

- RIP 是 64 位的指令指针，表示当前指令的地址。
- RIP 相对寻址的目标地址是通过：

  `目标地址 = 下一条指令地址（RIP） + 32 位偏移量`

- 这个偏移量是一个 有符号整数，所以它的范围是：

  从 −2³¹ 到 +2³¹−1，即 **±2GB** 的寻址范围。

这意味着，当前指令附近 ±2GB 范围内的任何数据都可以通过 RIP 相对寻址访问。


|        优点        |                          说明                         |
|:------------------:|:-----------------------------------------------------:|
| ✅ 节省空间         | 只用 4 字节表示偏移，比使用完整 64 位地址节省指令长度 |
| ✅ 支持位置无关代码 | 编译器只需计算偏移，不依赖绝对地址                    |
| ✅ 高效             | CPU 执行时只需加法运算，无需查表或重定位              |
| ✅ 安全             | 支持地址空间布局随机化（ASLR），提高安全性            |


### 为什么可以被多个进程共享？

因为代码中不再硬编码具体地址，多个进程可以：

- 使用同一份物理内存中的代码段。
- 每个进程有自己的数据段，但共享同一份只读代码。

这大大节省了内存，提高了系统效率。

举个例子：

| 进程 | 加载地址 |  使用的代码段  |
|:----:|:--------:|:--------------:|
| A    | 0x400000 | 使用共享代码段 |
| B    | 0x500000 | 使用共享代码段 |

两者的代码段内容完全一样，因为里面的寻址是相对 RIP 的，不依赖于加载地址。

为什么绝对寻址不可以被多进程共享？

- 每个进程的虚拟地址空间是独立的
  - 操作系统为每个进程分配独立的虚拟地址空间。
  - 即使两个进程都加载了同一个程序，它们的地址空间可能完全不同。
  - 如果代码中使用绝对地址，加载到不同地址空间后，这些地址就不再有效。

所以，绝对地址在一个进程中是有效的，在另一个进程中可能就指向错误的地方或根本不存在。

- 需要重定位，无法直接共享物理页
  - 如果使用绝对地址，操作系统必须在每个进程加载时对代码进行“重定位”，修改指令中的地址。
  - 一旦修改，代码段就变成了进程私有，不能共享同一份物理内存。
  - 而位置无关代码（如使用 RIP 相对寻址）不需要修改，可以直接映射到多个进程的地址空间。

- 违反共享库的设计原则
  - 动态链接库（如 `.so` 或 `.dll`）的核心优势就是可以被多个进程共享。
  - 如果库中使用绝对地址，每个进程都要有自己的副本，失去了共享的意义。
  - 正确做法是使用位置无关代码（PIC），让库在任意地址都能运行。


|    区域    | 是否可共享 |                  原因说明                 |
|:----------:|:----------:|:-----------------------------------------:|
| 代码段     | ✅ 是       | 只读 + 位置无关，多个进程可映射同一物理页 |
| 数据段     | ❌ 否       | 每个进程的数据不同，需独立副本            |
| 堆         | ❌ 否       | 动态分配，地址空间不同                    |
| 栈         | ❌ 否       | 私有调用栈，不能混用                      |
| 共享内存段 | ✅ 是       | 显式创建，专门用于共享                    |


如果你想深入了解某个进程的内存布局，可以分析 `/proc/[pid]/maps` 或用工具如 `pmap`、`vmmap`。

## 实际 debug 例子：在多线程中查看 poll 的事件

先复习下 `poll` 函数：

```c
int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// 第一个参数 fds 的类型
struct pollfd {
    int   fd;         /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};
```

```bash
# 查看 polll 的汇编代码
(gdb) disass poll
Dump of assembler code for function poll:
   0x00002ac084b5ec10 <+0>:     cmpl   $0x0,0x2d939d(%rip)        # 0x2ac084e37fb4 <__libc_multiple_threads>
   0x00002ac084b5ec17 <+7>:     jne    0x2ac084b5ec29 <poll+25>
   0x00002ac084b5ec19 <+0>:     mov    $0x7,%eax
   0x00002ac084b5ec1e <+5>:     syscall 
   0x00002ac084b5ec20 <+7>:     cmp    $0xfffffffffffff001,%rax
   0x00002ac084b5ec26 <+13>:    jae    0x2ac084b5ec59 <poll+73>
   0x00002ac084b5ec28 <+15>:    ret    
   0x00002ac084b5ec29 <+25>:    sub    $0x8,%rsp
   0x00002ac084b5ec2d <+29>:    call   0x2ac084b77600 <__libc_enable_asynccancel>
   0x00002ac084b5ec32 <+34>:    mov    %rax,(%rsp)
   0x00002ac084b5ec36 <+38>:    mov    $0x7,%eax
   0x00002ac084b5ec3b <+43>:    syscall 
   0x00002ac084b5ec3d <+45>:    mov    (%rsp),%rdi
   0x00002ac084b5ec41 <+49>:    mov    %rax,%rdx
   0x00002ac084b5ec44 <+52>:    call   0x2ac084b77660 <__libc_disable_asynccancel>
   0x00002ac084b5ec49 <+57>:    mov    %rdx,%rax
   0x00002ac084b5ec4c <+60>:    add    $0x8,%rsp
   0x00002ac084b5ec50 <+64>:    cmp    $0xfffffffffffff001,%rax
   0x00002ac084b5ec56 <+70>:    jae    0x2ac084b5ec59 <poll+73>
=> 0x00002ac084b5ec58 <+72>:    ret    
   0x00002ac084b5ec59 <+73>:    mov    0x2d31f0(%rip),%rcx        # 0x2ac084e31e50
   0x00002ac084b5ec60 <+80>:    neg    %eax
   0x00002ac084b5ec62 <+82>:    mov    %eax,%fs:(%rcx)
   0x00002ac084b5ec65 <+85>:    or     $0xffffffffffffffff,%rax
   0x00002ac084b5ec69 <+89>:    ret

# 找到所有的 ret 指令，设置条件断点
# 注意：最好是在 ret 指令之前的指令上也加上断点，
# 因为 ret 的时候，可能已经把当前栈（除 rsp / rbp 外）都弹出了，寄存器中将看不到当前栈的信息
#
# $rax 是返回值寄存器，也就是返回大于 0 时，让进程暂停
# 这里的 * 表示取内存的值（存放的是指令），* 断不可少，不然会被认为是 Function name
#
(gdb) b *0x00002ac084b5ec26 thread 4 if $rax > 0
(gdb) b *0x00002ac084b5ec28 thread 4 if $rax > 0
(gdb) b *0x00002ac084b5ec56 thread 4 if $rax > 0
(gdb) b *0x00002ac084b5ec58 thread 4 if $rax > 0
(gdb) b *0x00002ac084b5ec65 thread 4 if $rax > 0
(gdb) b *0x00002ac084b5ec69 thread 4 if $rax > 0

# 继续运行
(gdb) c

# 当 IO 事件发生，程序会被暂停

```

