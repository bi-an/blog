---
title: 编译原理
date: 2024-01-23 21:36:08
categories: c/cpp
tags: compile
---

## 编译主要步骤

参考[博客](https://blog.csdn.net/chen1415886044/article/details/104537547)

1. 预处理

``` bash
gcc -E test.c -o test.i
```

2. 编译

    编译是将高级语言（例如C、C++、Jave等）代码转换成机器码的过程。
    编译可以分成多个阶段，包括词法分析、语法分析、语义分析、优化和代码生成。
    编译器首先将源代码转换一种中间表示（通常是汇编代码或字节码），然后再将其转换为目标机器的机器代码。
    经过编译的代码通常是二进制的，可以直接在目标机器上执行。

先生成汇编代码：

``` bash
gcc -S test.i -o test.s
```

3. 汇编

    将汇编代码转成机器码。

``` bash
gcc -c test.s -o test.o
```

4. 链接

    该目标文件与其他目标文件、库文件、启动文件等链接起来生成可执行文件。

``` bash
gcc test.o -o test
```

Note: 可以使用 `-save-temps` 选项以保留编译过程中的所有中间文件：

```bash
gcc -save-temps test.c
```

## 链接

## C++名称修饰

`Name mangling (C++ only)`: 名称修饰，也称为名称重整、名称改编。参见链接[1](https://www.ibm.com/docs/en/i/7.2?topic=linkage-name-mangling-c-only)、[2](https://zh.wikipedia.org/wiki/%E5%90%8D%E5%AD%97%E4%BF%AE%E9%A5%B0)。

### 环境变量

参考：`man ld.so`

    PATH
    LD_LIBRARY_PATH
    LD_PRELOAD

### 函数

### 命令

参考：`man ld.so`, `man vdso`, `man elf`, [scanelf](https://linux.die.net/man/1/scanelf)

    ld(1), ldd(1), pldd(1), sprof(1), dlopen(3),getauxval(3), elf(5), capabilities(7),
    rtld-audit(7), ldconfig(8), sln(8), vdso(7), as(1), elfedit(1), gdb(1), nm(1),
    objcopy(1), objdump(1), patchelf(1), readelf(1), size(1), strings(1), strip(1),
    execve(2), dl_iterate_phdr(3), core(5), ld.so(8)

ldconfig

    配置动态连接器（dynamic linker）的运行时绑定（dynamic bindings）。
    如果你刚刚安装好共享库，可能需要运行ldconfig：
        sudo ldconfig
    通常需要超级用户来运行ldconfig，因为可能对需要某些root用户所有的目录和文件有写入权限。
    lddconfig 为从以下目录找到的共享库创建必要的 links 和 cache ：
        command line指定的目录；
        /etc/ld.so.conf文件中指定的目录；
        受信任的目录: /lib, /lib64, /usr/lib, /usr/lib64 。
    该 cache 被运行时连接器（run-time linker） ld.so 或 ld-linux.so 使用。

    ldconfig 尝试基于该库连接的 C 库来推断 ELF 库（比如 libc5 或 libc6/glibc）的类型。

    一些现有的库没有包含足够的信息来推断其类型。
    因此， /etc/ld.so.conf 文件格式允许指定期望的类型。
    这只在这些 ELF 库不能被解决的情况下使用。

    ldconfig 期望的符号链接有某种特定的形式，比如：

        libfoo.so -> libfoo.so.1 -> libfoo.so.1.12

    其中，中间的文件 libfoo.so.1 是库的 SONAME 。

    如果不遵循这种格式可能会导致升级后的兼容性问题。

ldd

    描述：
        ldd调用标准动态连接器（见 ld.so(8)），并且将环境变量 LD_TRACE_LODADED_OBJECTS 为 1 。
        这会让动态连接器检查程序的动态依赖，并且寻找（根据 ld.so(8) 描述的规则）
        和加载满足这些依赖的目标。对于每一条依赖，
        ldd 显示匹配的目标的位置和其载入处的16进制地址。
        （linux-vdso和ld-linux共享依赖是特殊的；见vdso(7)和ld.so(8)）

    安全性：
        注意，在某些情况下，一些版本的ldd可能会尝试通过直接运行程序（可能导致程序中的ELF解释器
        或程序本身的运行）来获取依赖信息。

        因此，永远不要在不受信任的可执行文件上使用ldd，因为会导致随意代码的运行。更安全替代方法为：
            $ objdump -p /path/to/program | grep NEEDED
        注意，这种替代方法只会显示该可执行文件的直接依赖，而ldd显示该可执行文件的整个依赖树。

    解释ldd的输出:
    $ ldd -v libibsupport_real.so 
    ./libibsupport_real.so: /usr/lib64/libibverbs.so.1: version `IBVERBS_1.8' not found (required by ./libibsupport_real.so)
        linux-vdso.so.1 =>  (0x00002ad3e49e3000)
        libibverbs.so.1 => /usr/lib64/libibverbs.so.1 (0x00002ad3e589b000)
        ...

        Version information:
        ./libibsupport_real.so:
                libgcc_s.so.1 (GCC_3.0) => /usr/lib64/libgcc_s.so.1
                libibverbs.so.1 (IBVERBS_1.8) => not found
                libibverbs.so.1 (IBVERBS_1.1) => /usr/lib64/libibverbs.so.1
                ...
        /usr/lib64/libnl-3.so.200:
                    libm.so.6 (GLIBC_2.2.5) => /usr/lib64/libm.so.6
                    ...
    在"Version information"中，"libgcc_s.so.1 (GCC_3.0) => /usr/lib64/libgcc_s.so.1"表示：
        "libgcc_s.so.1"指定一个shared library的名字（libgcc_s.so.1是GCC runtime library的一部分），该shared library为"./libibsupport_real.so"所依赖；
        "(GCC_3.0)"表明"libgcc_s.so.1"需要3.0版本及以上的GNU Compiler Collection (GCC)；
        "=>"指出满足依赖的shared library；
        "/usr/lib64/libgcc_s.so.1"是满足要求的shared library的路径。

  [ldd output说明](https://stackoverflow.com/questions/34428037/how-to-interpret-the-output-of-the-ldd-program)

sprof

参考：[stackoverflows](https://stackoverflow.com/questions/881074/how-to-use-sprof)

objdump

    [-p|--private-headers]
    [-x|--all-headers]

readelf

    [-d|--dynamic]

### 文件

    /lib/ld.so
        Run-time linker/loader.
    /etc/ld.so.conf
        File containing a list of directories, one per line, in which to search for libraries.
    /etc/ld.so.cache
        File containing an ordered list of libraries found in the directories specified in /etc/ld.so.conf, as well as those found in the trusted directories.

    The trusted directories:
        /lib
        /lib64
        /usr/lib
        /usr/lib64

ld.so

    名字
        ld.so, ld-linux.so - 动态连接/加载器。

    简介
        动态连接器可以被间接运行或直接运行。
        间接运行：
            运行某些动态连接程序或共享库。在这种情况下，不能向动态连接器传递命令行选项；并且在ELF情况下，存储在程序的.interp section中的动态连接器被执行。
        直接运行：
            /lib/ld-linux.so.* [OPTIONS] [PRAGRAM [ARGUMENTS]]

    描述
        ld.so 和 ld-linux.so* 寻找和加载程序所需的共享对象（共享库），准备程序的运行，然后运行它。

        如果在编译期没有向 ld(1) 指定 -static 选项，则Linux二进制文件需要动态连接（在运行时连接）。


### SONAME

参考：`man ldconfig`
参考：[SONAME Wiki](https://en.wikipedia.org/wiki/Soname)

> GNU linker使用 -hname 或 -soname=name 来指定该库的library name field。
> 在内部，linker会创建一个 DT_SONAME field并且用 name 来填充它。

1. 指定SONAME：

```bash
#Use ld:
$ ld -shared -soname libexample.so.1 -o libexample.so.1.2.3 file1.o file2.o
#Use gcc or g++:
$ gcc -shared -Wl,-soname,libexample.so.1 -o libexample.so.1.2.3 file1.o file2.o
```

2. 安装时创建软链接：

```bash
ln -s libexample.so.1.2.3 libexample.so.1
ln -s libexample.so.1 libexample.so
```

3. 查看SONAME：

```bash
#Use objdump
$ objdump -p libexample.so.1.3 | grep SONAME
  SONAME               libexample.so.1
#Use readelf
$ readelf -d libexample.so | grep SONAME
 0x000000000000000e (SONAME)             Library soname: [libexample.so.1]
```

### 符号版本控制（symbol versioning）

#### 定制函数版本：

1. example.c中定义函数`my_printf`：

```cpp
#include <stdio.h>

void my_printf(const char* format) {
    printf("%s", format);
}
```

2. version_script.txt中定义函数的版本为`VERSION_1`：

```text
VERSION_1 {
 global:
   my_printf;
 local:
   *;
};
```

3. 编译时指定`version_script.txt`为version-script：

```bash
gcc -shared -Wl,--version-script=version_script.txt -o libexample.so example.o
```

其中，`-Wl,`引出连接器选项。

4. 查看：
   1. 使用`readelf -sW libexample.so`查看函数`my_printf`的版本号（"VERSION_1"）：

```text
Symbol table '.dynsym' contains 8 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTable
     2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.2.5 (3)
     3: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
     4: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
     5: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@GLIBC_2.2.5 (3)
     6: 0000000000001105    39 FUNC    GLOBAL DEFAULT   13 my_printf@@VERSION_1
     7: 0000000000000000     0 OBJECT  GLOBAL DEFAULT  ABS VERSION_1
```

   2. 使用`objdump -T libexample.so`
   3. 使用`nm -D libexample.so`

#### 实现同一个函数有多个版本

1. 编写源代码

```cpp
#include <stdio.h>

void foo() {
    printf("foo version 1.0\n");
}

void foo_v2() {
    printf("foo_v2 version 2.0\n");
}
```

2. 编写版本脚本

例如，version_script.map文件如下：

```bash
VERS_1.0 {
    global: # global表示符号全局可见；默认为局部（也可能通过local显式声明为局部），不会被导出
        foo;
} VERS_1.1;

VERS_1.1 {
    global:
        foo_v2;
    foo; # VER_1.1继承自VER_1.0
         # 1. foo在VER_1.0中已经声明为了global，不需要再次声明global
         # 2. 当然可以再次显式声明为global，但是这不是推荐的做法
         # 3. 假设foo在VER_1.0中不是global，此处声明为global，
         # 会将原本不是global的foo变得全局可见
         # 4. 我们其实无需再次导出foo，因为除非我们显式隐藏（使用local关键字），foo就在
         # VER_1.1是和VER_1.0中的可见性保持一致
         # 5. 如果希望在VER_1.1中的foo与VER_1.0有不同的行为，那么需要再次将其导出
} VERS_2.0;

VERS_2.0 {
    global:
        foo_v2
};
```

VER_1.1继承了VER_1.0的全部符号，同时VER_1.1可以增加或修改符号。

VER_1.0：第一个版本，导出foo符号；
VER_1.1：第二个版本，引入foo_v2，并重新导出foo；


#### 程序依赖的库版本

有如下简单代码：

```cpp
// file: main.cc
int main() {
        return 0;
}
```

正常编译：

```bash
g++ main.cc -o a.out
```

查看符号：

```bash
strings a.out
```

你会发现其中有一些版本信息：

```
...
GLIBC_2.2.5
GLIBC_2.34
...
```

这是因为编译器和链接器生成库或可执行文件时，会根据系统上安装的glibc版本（因为库和可执行文件依赖glibc），
为库或可执行文件的函数符号附加上特定的版本信息。
为了保证向下兼容，glibc通过符号控制保留了多个版本的符号，支持老版本的软件能在较新的系统上运行。
例如，如果某个函数在glibc2.2.5中引入，它可以继续保留在之后的版本中，但符号标记为GLIBC_2.2.5。

### ELF

    ELF - Executable and Linking Format.
    ELF描述了normal executable files、relocatable object files、core files和shared objects的格式。

参考：`man elf`, [博客:elf介绍](http://chuquan.me/2018/05/21/elf-introduce/)。

### 链接名

参考：[程序员的自我修养](https://littlebee1024.github.io/learning_book/booknotes/cxydzwxy/link/dynamic/#_16), `man ld`

GCC的提供了不同的方法指定链接的共享库：

- `l<link_name>`参数

    指定需要链接的共享库lib<link_name>.so

- `l:<filename>`参数

    通过文件名指定共享库，参考LD手册

- 全路径指定

- `Wl,-static`参数

    指定查找静态库，通过-Wl,-Bdynamic恢复成动态库查找

### 链接器选项

参考：`man ld`

    ld - The GNU linker

选项：

    -rpath=dir
        添加一个目录到运行时库搜寻路径中。

gcc通过 `-Wl` 前缀指定链接器选项，例如：

     gcc -Wl,--start-group foo.o bar.o -Wl,--end-group
     gcc -Wl,-rpath,'$ORIGIN/../lib'


### Tips

1. [How to find out the dynamic libraries are loaded when running an executable?](https://unix.stackexchange.com/questions/120015/how-to-find-out-the-dynamic-libraries-executables-loads-when-run)

Answer:

- `ldd /path/to/program`
- `objdump -p /path/to/program | grep NEEDED`
- `lddtree` (from `pax-utils`) or `readelf -d /bin/ls | grep 'NEEDED'`
- `lsof -p PID | grep mem`

```bash
$ pidof nginx
6920 6919

$ lsof -p 6919 | grep mem
```

- `strace -e trace=open myprogram`

`ldd` and `lsof` show the libraries loaded either directly or at a given moment. They do not account for libraries loaded via `dlopen` (or discarded by `dlclose`). You can get a better picture of this using `strace`.

- `pmap <pid> -p`

### Notice

1. 生成so时，链接器不会寻找其so依赖；executable会寻找so的依赖关系。换句话说，即使so生成过程不报错，但是executable生成时可能会报错。
2. 生成so时，如果引用了其他 so ，只要 include 其他 so 的头文件，引用头文件中的符号时，就能编译通过。但是这样生成的 so 没有加上对应符号的依赖。
   此时用 readelf -s 查看对应的符号，其 type 为 NOTYPE 。

    ```text
    $ readelf -sW libb.so

    Symbol table '.dynsym' contains 14 entries:
    Num:    Value          Size Type    Bind   Vis      Ndx Name
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND _Z1fv
    ```

3. 如果加上对应 -la 选项，就会从 liba.so 中寻找依赖的符号，如果找到，则添加进入 so 的依赖项中。当运行时则会加载 liba.so ，否则运行时不会加载。
   用 readelf -s 查看对应的符号，其 type 不再时 NOTYPE ，而是 FUNC （如果该符号是一个函数的话）。

    ```text
     6: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND _Z1fv
    ```

   如果没有找到符号，或者说 liba.so 中根本没有实现这个函数，那么其 type 依然是 NOTYPE ，此时也不会报错。
   注意：如果只是 include 头文件，也就说只有某个符号的声明，并没有其定义或引用它，那么 so 中不会生成其信息。
