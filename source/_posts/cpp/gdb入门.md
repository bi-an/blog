---
title: gdb 入门
date: 2024-01-23 21:44:37
categories: c/cpp
tags: gdb
---

## gdb实现原理

参考[链接](https://www.zhihu.com/people/bi-an-60-46)。

## 在 vim / emacs 中启动 gdb

为什么在 Emacs/vim 中运行 GDB？

- 统一环境：你可以在 Emacs 中同时查看源代码和调试信息
- 快捷键支持：使用 Emacs 的快捷键控制 GDB（如设置断点、单步执行）
- 图形化布局：支持多窗口显示，如断点列表、堆栈信息、变量值等
- 增强效率：无需离开编辑器即可完成调试任务

Vim 从 8.1 版本开始内置了一个叫做 Termdebug 的插件，它可以直接在 Vim 中运行 GDB，并显示调试信息。

✅ 步骤：
确保 Vim 版本 ≥ 8.1

编译你的程序（加上 -g 选项）：

```bash
gcc -g my_program.c -o my_program
```

在 Vim 中加载插件：

```vim
:packadd termdebug
:Termdebug
```

Vim 会打开一个新的窗口，显示 GDB 控制台，你可以在里面输入 GDB 命令。

📖 教程参考：[Baeldung 的 Vim-GDB 集成指南](https://www.baeldung.com/linux/vim-gdb-integration)


## gdb命令

* `thread apply [threadno] [all] args` - 将命令传递给一个或多个线程，参见[链接](https://developer.apple.com/library/archive/documentation/DeveloperTools/gdb/gdb/gdb_5.html)。
比如，`thread apply all continue`表示将`continue`命令传递给所有线程，也就是让所有线程都继续运行。

* `rbreak` - Set a breakpoint for all functions matching REGEXP. 参考[链接](https://blog.csdn.net/zdl1016/article/details/8708077)。

    e.g. `rbreak file.C:.*` - 给file.C的所有函数加上断点。

* `info`
  - `info inferior` - 可以查看当前调试的进程的PID。另外一种方法是在gdb命令行中直接调用C函数：`print (int)getpid()`。参考：[链接](https://www.qiniu.com/qfans/qnso-36704270)。
  - `info source` - 当前调试的源文件路径。
  - `info sources <pattern>` - 查询源文件路径
  - `info proc` - [当前进程信息](https://sourceware.org/gdb/onlinedocs/gdb/Process-Information.html)。
    - `info proc files` - 当前进程打开的文件（和文件描述符）。
    - `info args` - 查看函数参数
    - `info locals` - 查看局部变量。
    - `info reg` 查看寄存器。

常用寄存器：

| 寄存器 |                     用途说明                    |
|:------:|:-----------------------------------------------:|
| rax    | 函数返回值，乘除法运算                          |
| rbx    | 通用寄存器，常用于基址                          |
| rcx    | 循环计数器                                      |
| rdx    | I/O 指针或中间数据                              |
| rdi    | 函数第一个参数                                  |
| rsi    | 函数第二个参数                                  |
| rsp    | 栈顶指针                                        |
| rbp    | 栈底指针，栈帧基址                              |
| rip    | 当前执行指令地址                                |
| eflags | 运算状态标志，如 ZF（零标志）、CF（进位标志）等 |

gdb 的寄存器变量

x86（32 位）

| $eax, $ebx, $ecx, $edx | 通用寄存器                       |
|------------------------|----------------------------------|
| $esi, $edi             | 源/目标索引寄存器                |
| $esp                   | 栈顶指针（Stack Pointer）        |
| $ebp                   | 栈底指针（Base Pointer）         |
| $eip                   | 指令指针（下一条执行的指令地址） |
| $eflags                | 标志寄存器（保存条件标志）       |

x86_64（64 位）

|         寄存器         |         说明        |
|:----------------------:|:-------------------:|
| $rax, $rbx, $rcx, $rdx | 通用寄存器（64 位） |
| $rsi, $rdi             | 参数寄存器          |
| $rsp                   | 栈顶指针            |
| $rbp                   | 栈底指针            |
| $rip                   | 指令指针            |
| $r8~$r15               | 扩展通用寄存器      |
| $eflags                | 标志寄存器          |

* `show`
  * `show environment` 查看全局变量
  * `set environment <var>=<value>` 设置环境变量
* `attach` - 连接到正在运行的进程。与`gdb -p`效果相同。
* `detach` - 取消连接的进程。
* `handle <signal> print pass nostop` - 捕获信号（比如`SIGSEGV`）并且忽略它。`handle <signal nostop`。
  * `pass` 表示 gdb 会将捕获到的信号发回给被调试的进程。
* `set` - 修改变量的值，比如`set x=10`（或`set var x=10`）将变量`x`的值改为`10`。参考[博客](https://blog.csdn.net/yasi_xi/article/details/12784507)。
* `show directories`
* `print` - gdb默认设置打印字符串的长度为200；更改打印最大长度：`set print elements <number-of-elements>`，`0`表示unlimited.
          - 打印数组: `print arr[0]@3` ，其中 `@3` 表示打印 3 个元素。
          - 以十六进制打印：`p/x <var>`
* `ptype <variable name>` - 打印变量类型。
* `finish` - 从函数中返回，并打印函数返回值（即使函数的return语句很复杂，也可以获取返回值）。
* `frame <n>` - 跳转到某个栈帧。
* `up` 跳转到上一个栈帧
* `x/FMT`: `x` 表示 `examine` ，查看内存。
  * `/i` 表示 `instruction` ，即查看汇编指令。
  * `/g` 表示 `giant word` ，即每次查看 8 字节。
    * `x/g 0x400000` 查看地址 0x400000 处的 8 字节内容（以十六进制显示）
    * `x/4g $rsp` 查看当前栈指针 `$rsp` 指向的连续 4 个 8 字节值（共 32 字节）

| 命令 |                 说明                |
|:----:|:-----------------------------------:|
| x/g  | 默认十六进制显示 8 字节内容         |
| x/dg | 十进制显示 8 字节内容               |
| x/ug | 无符号十进制显示 8 字节内容         |
| x/tg | 二进制显示 8 字节内容               |
| x/fg | 浮点数格式显示 8 字节内容（double） |



## 环境变量

见[链接](https://www.irya.unam.mx/computo/sites/manuales/fce12/debugger/cl/commandref/gdb_mode/cmd_set_environm.htm)


## 断点

添加断点：

```
break file:line_no
```

查看断点：

```
info break
```

删除第2个断点：

```
delete 2
```


### 条件断点

参考：[博客](http://c.biancheng.net/view/8255.html)、[文档](https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_28.html)。

`break ... if cond`


### 观察断点

### 捕捉断点

```
try...catch
```

### 打印长度的限制

* Value sizes - 参考：[文档](https://sourceware.org/gdb/onlinedocs/gdb/Value-Sizes.html)

```bash
set max-value-size bytes
set max-value-size unlimited
```

* 打印字符长度限制

  gdb默认设置打印字符串的长度为200；更改打印最大长度：`set print elements`


## coredump

gdb命令：`gcore`。

[Reference](https://man7.org/linux/man-pages/man5/core.5.html)

## WSL无法使用gdb

WSL指Windows虚拟机。

[解决方法](https://github.com/microsoft/WSL/issues/8516)：

安装[PPA的daily build版本](https://launchpad.net/~ubuntu-support-team/+archive/ubuntu/gdb)

```bash
sudo add-apt-repository ppa:ubuntu-support-team/gdb
sudo apt update
sudo apt install gdb
```

## gdb attach 权限报错

This is due to kernel hardening in Linux; you can disable this behavior by `echo 0 > /proc/sys/kernel/yama/ptrace_scope` or by modifying it in `/etc/sysctl.d/10-ptrace.conf`.

[How to solve "ptrace operation not permitted" when trying to attach GDB to a process?](https://stackoverflow.com/questions/19215177/how-to-solve-ptrace-operation-not-permitted-when-trying-to-attach-gdb-to-a-pro)

## gdb debug forks

[Reference](https://www-zeuthen.desy.de/unix/unixguide/infohtml/gdb/Forks.html)

By default, when a program forks, gdb will continue to debug the parent process and the child process will run unimpeded.

If you want to follow the child process instead of the parent process, use the command set `follow-fork-mode`.

`set follow-fork-mode mode`
Set the debugger response to a program call of `fork` or `vfork`. A call to fork or vfork creates a new process. The mode argument can be:
`parent`
The original process is debugged after a fork. The child process runs unimpeded. This is the default.
`child`
The new process is debugged after a fork. The parent process runs unimpeded.
`ask`
gdb 会提示让你选择 `parent` 还是 `child` 。

`show follow-fork-mode`
Display the current debugger response to a fork or vfork call.
On Linux, if you want to debug both the parent and child processes, use the command set detach-on-fork.

`set detach-on-fork mode`
Tells gdb whether to detach one of the processes after a fork, or retain debugger control over them both.
`on`
The child process (or parent process, depending on the value of follow-fork-mode) will be detached and allowed to run independently. This is the default.
`off`
Both processes will be held under the control of gdb. One process (child or parent, depending on the value of follow-fork-mode) is debugged as usual, while the other is held suspended.


`show detach-on-fork`
Show whether detach-on-fork mode is on/off.

If you issue a run command to gdb after an exec call executes, the new target restarts. To restart the parent process, use the file command with the parent executable name as its argument. By default, after an exec call executes, gdb discards the symbols of the previous executable image. You can change this behaviour with the set follow-exec-mode command.

set follow-exec-mode mode
Set debugger response to a program call of exec. An exec call replaces the program image of a process.
follow-exec-mode can be:

`new`
gdb creates a new inferior and rebinds the process to this new inferior. The program the process was running before the exec call can be restarted afterwards by restarting the original inferior.
For example:

```
(gdb) info inferiors
(gdb) info inferior
  Id   Description   Executable
* 1    <null>        prog1
(gdb) run
process 12020 is executing new program: prog2
Program exited normally.
(gdb) info inferiors
  Id   Description   Executable
* 2    <null>        prog2
  1    <null>        prog1
```

`same`
gdb keeps the process bound to the same inferior. The new executable image replaces the previous executable loaded in the inferior. Restarting the inferior after the exec call, with e.g., the run command, restarts the executable the process was running after the exec call. This is the default mode.
For example:

```
(gdb) info inferiors
  Id   Description   Executable
* 1    <null>        prog1
(gdb) run
process 12020 is executing new program: prog2
Program exited normally.
(gdb) info inferiors
  Id   Description   Executable
* 1    <null>        prog2
```

## Setting Catchpoints

[Reference](https://www-zeuthen.desy.de/unix/unixguide/infohtml/gdb/Set-Catchpoints.html#Set-Catchpoints)

## gdb redirect to a log file

You need to enable logging:

```
(gdb) set logging on
Now GDB will log to ./gdb.txt. You can tell it which file to use:

(gdb) set logging file my_god_object.log
And you can examine the current logging configuration:

(gdb) show logging
```

更新：新版本 gdb 采用 `set logging enabled on` 。

记录输入的命令：

```
(gdb) set trace-commands on
```

[Refercence](https://stackoverflow.com/questions/5941158/gdb-print-to-file-instead-of-stdout)


## 权限限制

```bash
$ cat /proc/sys/kernel/yama/ptrace_scope
3
```

Yama 是 Linux 内核中的一个 安全模块（LSM：Linux Security Module），专门用于加强进程间的访问控制，尤其是对 ptrace 系统调用的限制。
Yama 的主要目的是防止恶意程序通过 ptrace 附加到其他进程，从而窃取数据或注入代码。

| 值 |     模式名称     |                                      含义说明                                      |
|:--:|:----------------:|:----------------------------------------------------------------------------------:|
| 0  | 经典模式         | 允许同一用户调试其权限范围内的任意进程（只要目标进程是“可转储”的）。适合开发环境。 |
| 1  | 受限模式（默认） | 只允许调试直接子进程，或拥有 CAP_SYS_PTRACE 权限的进程。更安全，适合大多数系统。   |
| 2  | 管理员模式       | 只有 root 或具备 CAP_SYS_PTRACE 的进程可以使用 ptrace。适合高安全场景。            |
| 3  | 完全禁用         | 所有进程（包括 root）都无法使用 ptrace。彻底禁止调试行为，适合极端安全需求。       |

