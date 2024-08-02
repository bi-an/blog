---
title: Compilation Principle
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

    翻译成汇编代码。

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

### versioning information

定制函数版本：

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

4. 使用`readelf -sW libexample.so`查看函数`my_printf`的版本号（"VERSION_1"）：

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

