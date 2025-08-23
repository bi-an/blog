---
title: 汇编与 gdb
date: 2025-08-22 15:42:27
categories: c/cpp
tags:
---

## 1. 前言

我们在调试 release 版本的程序时，由于缺乏符号信息，所以需要通过寄存器来查看函数的参数、返回值等。

## 2. 寄存器

### 2.1. 通用寄存器 (General Purpose Registers)

| 寄存器名 | 英文名称          | 作用                                                                                              |
| -------- | ----------------- | ------------------------------------------------------------------------------------------------- |
| rax      | Accumulator       | 累加器，通常用于算术运算和函数返回值存储。                                                        |
| rbx      | Base              | 基址寄存器，常用于存储数据或指针。                                                                |
| rsi      | Source Index      | 源索引寄存器，常用于字符串操作中的源地址指针（函数第一个参数）。                                  |
| rdi      | Destination Index | 目标索引寄存器，常用于字符串操作中的目标地址指针或结构体指针（函数第二个参数）。                  |
| rdx      | Data              | 数据寄存器，常用于 I/O 操作或乘除法运算中的扩展数据存储（函数第三个参数）。                       |
| rcx      | Counter           | 计数器寄存器，常用于循环计数或字符串操作中的计数（函数第四个参数）。                              |
| rsp      | Stack Pointer     | 栈指针寄存器，指向当前栈顶。                                                                      |
| rbp      | Base Pointer      | 基址指针寄存器，指向当前栈帧的基址。                                                              |
| r8~r15   | General Purpose   | 通用寄存器，扩展的 64 位寄存器之一，用于存储数据或指针（`r8`~`r9` 常用于保存函数第五~六个参数）。 |

### 2.2. 特殊用途寄存器 (Special Purpose Registers)

| 寄存器名 | 英文名称            | 作用                                                   |
| -------- | ------------------- | ------------------------------------------------------ |
| rip      | Instruction Pointer | 指令指针寄存器，存储当前执行指令的地址。               |
| rflags   | Flags               | 标志寄存器，存储状态标志位（如进位、溢出、零标志等）。 |


### 2.3. 段寄存器 (Segment Registers)

| 寄存器名 | 英文名称      | 作用                                     |
| -------- | ------------- | ---------------------------------------- |
| cs       | Code Segment  | 代码段寄存器，指向当前代码段的基址。     |
| ds       | Data Segment  | 数据段寄存器，指向当前数据段的基址。     |
| es       | Extra Segment | 额外段寄存器，指向额外数据段的基址。     |
| fs       | FS Segment    | 特殊用途段寄存器，常用于线程本地存储等。 |
| gs       | GS Segment    | 特殊用途段寄存器，常用于线程本地存储等。 |
| ss       | Stack Segment | 栈段寄存器，指向当前栈段的基址。         |

### 2.4. 浮点与向量寄存器 (Floating Point and Vector Registers)

| 寄存器名   | 英文名称          | 作用                                   |
| ---------- | ----------------- | -------------------------------------- |
| xmm0-xmm15 | SIMD Registers    | 用于 SSE 指令集的 128 位向量运算。     |
| ymm0-ymm15 | AVX Registers     | 用于 AVX 指令集的 256 位向量运算。     |
| zmm0-zmm31 | AVX-512 Registers | 用于 AVX-512 指令集的 512 位向量运算。 |

### 2.5. 函数调用时的参数传递

在 x86_64 架构中，函数调用时的参数传递遵循 System V AMD64 ABI（Linux/Unix 系统的标准调用约定）。

前六个整数或指针类型的参数依次存储在以下寄存器中：
1. **rdi** - 第一个参数
2. **rsi** - 第二个参数
3. **rdx** - 第三个参数
4. **rcx** - 第四个参数
5. **r8**  - 第五个参数
6. **r9**  - 第六个参数

对于浮点类型的参数（如 `float` 或 `double`），前八个参数存储在以下 **SSE 寄存器** 中：
1. **xmm0** - 第一个浮点参数
2. **xmm1** - 第二个浮点参数
3. **xmm2** - 第三个浮点参数
4. **xmm3** - 第四个浮点参数
5. **xmm4** - 第五个浮点参数
6. **xmm5** - 第六个浮点参数
7. **xmm6** - 第七个浮点参数
8. **xmm7** - 第八个浮点参数

溢出参数（超过寄存器数量）会依次存储在 **栈** 中：
- 超过寄存器数量（整数参数超过 6 个，浮点参数超过 8 个）的参数会依次压入栈中。
- 栈需要保持 16 字节对齐，可能会插入填充字节。
- 可以通过访问栈指针（rsp）或基址指针（rbp）来找到栈上的参数。
  - 使用 rsp（栈指针）
    - 在函数入口时，rsp 指向栈顶（即返回地址的下一个位置）。
    - 栈上的第一个参数位于 [rsp + 8]（跳过返回地址）。
    - 第二个参数位于 [rsp + 16]，依此类推。
  - 使用 rbp（基址指针）
    - 如果函数使用了帧指针（rbp），rbp 通常指向调用者的栈帧基址。
    - 栈上的第一个参数位于 [rbp + 16]（跳过返回地址和保存的 rbp）。
    - 第二个参数位于 [rbp + 24]，依此类推。

---

## 3. 栈

### 3.1. 理解栈布局

在函数调用时，栈的布局通常如下（从高地址到低地址）：

1. 返回地址：调用函数时，call 指令会将返回地址（下一条指令的地址）压入栈中。
2. 溢出参数：如果参数超过寄存器数量，多余的参数会依次压入栈中。
3. 栈对齐填充：为了满足 16 字节对齐要求，可能会有额外的填充字节。
4. 局部变量和保存的寄存器：函数内部可能会在栈上分配空间用于局部变量或保存调用者的寄存器。

### 3.2. 函数调用时的压栈过程

在x86_64架构中，函数调用时会涉及到栈的操作，包括压栈和出栈。这些操作主要用于保存调用者的上下文（如返回地址、寄存器值）以及为被调用函数分配栈帧。

#### 3.2.1. 调用者（Caller）的操作
1. **压入返回地址**  
   当调用者使用 `call` 指令调用函数时，CPU会自动将返回地址（下一条指令的地址）压入栈中。此时，`rsp`（栈指针）会减少8字节（64位系统）。

   ```asm
   call function
   # 等价于：
   push rip  ; 将返回地址压入栈
   jmp function
   ```

2. **压入溢出参数（如果有）**  
   如果函数的参数超过了寄存器数量（整数参数超过6个，浮点参数超过8个），多余的参数会从右到左依次压入栈中。`rsp` 会随着每个参数的压入减少。

3. **对齐栈**  
   为了满足 **16字节对齐** 的要求，调用者可能会插入额外的填充字节，使得 `rsp` 在调用函数前保持16字节对齐。


#### 3.2.2. **被调用者（Callee）的操作**
1. **保存调用者的栈帧基址**  
   被调用者通常会保存调用者的栈帧基址（`rbp`），以便在函数返回时恢复调用者的栈帧。  
   ```asm
   push rbp       ; 保存调用者的 rbp
   mov rbp, rsp   ; 设置当前函数的栈帧基址
   ```

2. **分配栈空间**  
   被调用者会根据函数内部局部变量的需求，在栈上分配空间。`rsp` 会减少相应的字节数。  
   ```asm
   sub rsp, <size>  ; 为局部变量分配栈空间
   ```


### 3.3. **2. 函数返回时的出栈过程**

#### 3.3.1. **被调用者（Callee）的操作**
1. **释放局部变量的栈空间**  
   被调用者在返回前会释放为局部变量分配的栈空间。  
   ```asm
   add rsp, <size>  ; 恢复 rsp
   ```

2. **恢复调用者的栈帧基址**  
   被调用者会恢复调用者的 `rbp`，以确保调用者的栈帧完整。  
   ```asm
   pop rbp  ; 恢复调用者的 rbp
   ```

3. **返回到调用者**  
   被调用者使用 `ret` 指令从栈中弹出返回地址，并跳转到该地址。  
   ```asm
   ret  ; 等价于：pop rip
   ```


#### 3.3.2. **调用者（Caller）的操作**
1. **清理栈上的参数（如果需要）**  
   如果调用约定要求调用者清理栈上的参数（如 `cdecl` 调用约定），调用者会调整 `rsp`。  
   ```asm
   add rsp, <size>  ; 清理栈上的参数
   ```


### 3.4. **3. 栈指针（`rsp`）和基址指针（`rbp`）的变化**

以下是一个函数调用的栈布局示例：

 **C代码**
```c
void example(int a, int b) {
    int x = a + b;
}

int main() {
    example(1, 2);
    return 0;
}
```

**汇编代码（简化版）**
```asm
# main 函数
main:
    sub rsp, 16         ; 对齐栈
    mov edi, 1          ; 第一个参数 -> rdi
    mov esi, 2          ; 第二个参数 -> rsi
    call example        ; 调用 example 函数
    add rsp, 16         ; 恢复栈
    ret

# example 函数
example:
    push rbp            ; 保存调用者的 rbp
    mov rbp, rsp        ; 设置当前栈帧基址
    sub rsp, 16         ; 为局部变量分配栈空间
    mov eax, edi        ; a -> eax
    add eax, esi        ; a + b
    leave               ; 恢复栈帧（等价于：mov rsp, rbp; pop rbp）
    ret                 ; 返回调用者
```

 **栈布局变化**
| 操作           | `rsp` 变化  | 栈内容（从高地址到低地址） |
| -------------- | ----------- | -------------------------- |
| `call example` | `rsp -= 8`  | 返回地址                   |
| `push rbp`     | `rsp -= 8`  | 保存调用者的 `rbp`         |
| `sub rsp, 16`  | `rsp -= 16` | 为局部变量分配空间         |
| `leave`        | `rsp += 16` | 释放局部变量空间           |
| `ret`          | `rsp += 8`  | 弹出返回地址               |


### 3.5. **总结**
1. **函数调用的栈操作**：
   - 调用者负责压入返回地址和溢出参数。
   - 被调用者负责保存 `rbp` 和分配局部变量空间。
   - 函数返回时，释放局部变量空间并恢复调用者的栈帧。
2. **`rsp` 和 `rbp` 的变化**：
   - `rsp` 指向栈顶，动态变化。
   - `rbp` 指向栈帧基址，通常固定不变。


## 4. 使用 gdb 查看寄存器

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
- `g`：表示每个单元的 size，有 b(byte), h(halfword), w(word), g(giant, 8 bytes)
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

## 5. 使用 gdb 查看栈

- bt
- frame
- args
- locals
- x


TDODO

### 5.1. `frame` 与寄存器的值

- GDB 中的寄存器值（如 `$rax`, `$rdi`, `$rsp` 等）是当前 CPU 执行上下文的快照。
- 当你切换到 `frame 0`（最内层栈帧）时，寄存器值是最真实的，因为这是程序当前正在执行的地方。
- 当你切换到 外层栈帧（frame 1, 2, ...）时，GDB 会尝试还原当时的寄存器状态，但这依赖于：
  - 编译器是否保存了寄存器值（如 callee-saved）
  - 是否有调试符号或 unwind 信息
  - GDB 是否能推断出寄存器的保存位置

寄存器值可能出现的情况

|                     情况                      |                  表现                  |
| :-------------------------------------------: | :------------------------------------: |
|   寄存器是 caller-saved（如 rdi, rsi, rax）   |     可能显示 <not saved> 或错误值      |
| 寄存器是 callee-saved（如 rbx, rbp, r12~r15） |             通常能正确还原             |
|            没有调试信息或优化严重             | GDB 无法还原，显示当前值或 <not saved> |

建议

- 如果你要分析寄存器状态，最好在 frame 0 或断点处进行。
- 如果你在分析 core dump 或栈破坏问题，寄存器值只能作为参考，不要完全依赖外层 frame 的寄存器快照。
- 使用 info args 和 info locals 更可靠地查看参数和局部变量（如果有符号信息）。

## 6. 在特定线程中设置断点

### 6.1. 断点只作用于某线程

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

### 6.2. 锁定调度器，只让当前线程运行

默认情况下，GDB 会让所有线程一起运行（比如你执行 continue 时）。如果你只想让当前线程运行，其它线程保持暂停，可以使用：

```bash
(gdb) set scheduler-locking on
```

这表示：只有当前线程会执行，其他线程全部暂停。

其中模式还有：

| 模式  |                         说明                          |
| :---: | :---------------------------------------------------: |
|  off  |              默认值，所有线程都可以运行               |
|  on   |            只有当前线程运行，其他线程暂停             |
| step  | 单步调试时只运行当前线程，continue 时其他线程也会运行 |

你可以随时切换：

```bash
(gdb) set scheduler-locking step
```

- 如果你在调试死锁、竞态或线程间通信问题，锁定调度器是非常有效的方式。
- 如果你在调试某个 poll() 或 epoll_wait() 调用，只想观察某个线程的行为，可以结合 catch syscall 和 thread 命令一起使用。


## 7. 查看汇编代码

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

## 8. 位置无关代码（PIC）


### 8.1. **什么是 PIC**
- **PIC**（Position Independent Code，位置无关代码）是一种编译方式，使得生成的代码可以在内存中的任意位置运行，而无需硬编码绝对地址。
- 在动态链接库（`shared libraries`）中，通常需要使用 PIC，以便库可以被加载到任意内存地址。
- 节省空间：相比使用 64 位绝对地址，RIP 相对寻址只需要一个 32 位偏移量。
- 更安全：支持地址随机化（ASLR），提高程序的安全性。

### 8.2. **PIC 的实现**
1. **访问全局变量**  
   在 PIC 模式下，代码通过 **全局偏移表（GOT, Global Offset Table）** 和 **过程链接表（PLT, Procedure Linkage Table）** 访问全局变量和函数地址。
   
2. **寄存器 `rip` 的使用**  
   x86_64 支持基于 `rip`（指令指针）的寻址方式，PIC 会利用 `rip` 相对寻址来访问全局变量或函数地址，而不是使用绝对地址。

在 x86-64 架构中，传统的绝对地址寻址方式不再适用于位置无关代码。于是引入了 RIP（指令指针）相对寻址：

假设你有一个全局变量 int x = 42;，在汇编中访问它可能会变成：

```asm
asm
mov eax, DWORD PTR [rip + offset_to_x]
```

这里的 offset_to_x 是编译器计算出来的 x 相对于当前指令的偏移量。


|    寻址方式    |                描述                 | 是否位置无关 |
| :------------: | :---------------------------------: | :----------: |
|  绝对地址寻址  |     使用固定地址，如 [0x400123]     |     ❌ 否     |
| 寄存器间接寻址 |     如 [rax]，地址由寄存器决定      |     ✅ 是     |
|  RIP 相对寻址  | 如 [rip + offset]，相对当前指令位置 |     ✅ 是     |

### 8.3. **PIC 的优化**
- **减少重定位**：通过 `rip` 相对寻址，避免了加载时的重定位操作，提高了加载速度。
- **共享内存**：多个进程可以共享同一段动态库代码，而无需为每个进程生成独立的副本。

### 8.4. **示例**
```asm
mov rax, [rip + global_var@GOTPCREL]  ; 通过 GOT 表访问全局变量
call [rip + func@PLT]                ; 通过 PLT 表调用函数
```

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


### 8.5. 为什么使用 RIP 相对寻址？

1. RIP 是唯一始终已知的寄存器
- 在执行指令时，CPU 总是知道当前指令的地址（即 RIP）。
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

### 8.6. 为什么使用 RIP 相对寻址只需要一个 32 位偏移量

在 x86-64 架构中，RIP 相对寻址的偏移量被设计为一个有符号的 32 位整数，也就是一个 displacement（位移）字段，它在机器码中只占用 4 个字节。

- RIP 是 64 位的指令指针，表示当前指令的地址。
- RIP 相对寻址的目标地址是通过：

  `目标地址 = 下一条指令地址（RIP） + 32 位偏移量`

- 这个偏移量是一个 有符号整数，所以它的范围是：

  从 −2³¹ 到 +2³¹−1，即 **±2GB** 的寻址范围。

这意味着，当前指令附近 ±2GB 范围内的任何数据都可以通过 RIP 相对寻址访问。


|        优点        |                         说明                          |
| :----------------: | :---------------------------------------------------: |
|     ✅ 节省空间     | 只用 4 字节表示偏移，比使用完整 64 位地址节省指令长度 |
| ✅ 支持位置无关代码 |          编译器只需计算偏移，不依赖绝对地址           |
|       ✅ 高效       |       CPU 执行时只需加法运算，无需查表或重定位        |
|       ✅ 安全       |      支持地址空间布局随机化（ASLR），提高安全性       |


### 8.7. 为什么可以被多个进程共享？

因为代码中不再硬编码具体地址，多个进程可以：

- 使用同一份物理内存中的代码段。
- 每个进程有自己的数据段，但共享同一份只读代码。

这大大节省了内存，提高了系统效率。

举个例子：

| 进程  | 加载地址 |  使用的代码段  |
| :---: | :------: | :------------: |
|   A   | 0x400000 | 使用共享代码段 |
|   B   | 0x500000 | 使用共享代码段 |

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


|    区域    | 是否可共享 |                 原因说明                  |
| :--------: | :--------: | :---------------------------------------: |
|   代码段   |    ✅ 是    | 只读 + 位置无关，多个进程可映射同一物理页 |
|   数据段   |    ❌ 否    |      每个进程的数据不同，需独立副本       |
|     堆     |    ❌ 否    |          动态分配，地址空间不同           |
|     栈     |    ❌ 否    |           私有调用栈，不能混用            |
| 共享内存段 |    ✅ 是    |          显式创建，专门用于共享           |


如果你想深入了解某个进程的内存布局，可以分析 `/proc/[pid]/maps` 或用工具如 `pmap`、`vmmap`。

## 9. 实际 debug 例子：在多线程中查看 poll 的事件

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

**FIXME：这种在汇编代码 ret 前断点，并依据 `rax` 、`rdi` 设置条件断点的方式不可靠，因为可能进入了 libc 层。**

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

