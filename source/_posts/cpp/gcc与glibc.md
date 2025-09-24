---
title: gcc 与 glibc
date: 2024-02-02 13:24:45
categories: c/cpp
tags:
    - gcc
    - glibc
---

## gcc

gcc是一个编译套件，包含c、c++、Fortran语言的编译器。

## glibc

glibc是一个library，为C程序提供基础公共功能，包括系统调用、数学函数和其他核心组件。
Linux平台和vscode似乎都依赖glibc，如果擅自将`LD_LIBRARY_PATH`更改为其他版本的glibc路径，则bash会直接crash。

### glibc包含以下bin和lib：

```bash
$ cd glibc-v2.34/Linux/RHEL7.0-2017-x86_64/bin && ls
catchsegv  getconf  iconv  locale     makedb  pcprofiledump  sotruss  tzselect  zdump
gencat     getent   ldd    localedef  mtrace  pldd           sprof    xtrace

# 进入其他版本的glibc/lib目录执行ls命令会报错，大概原因可能是因为当前路径的glibc的lib和系统的lib冲突。
$ cd ../lib && ls
ls: relocation error: ./libc.so.6: symbol __tunable_get_val, version GLIBC_PRIVATE not defined in file ld-linux-x86-64.so.2 with link time reference

$ cd .. && ls lib
Mcrt1.o               libanl.so.1             libm.so             libnss_hesiod.so.2
Scrt1.o               libc.a                  libm.so.6           libpcprofile.so
audit                 libc.so                 libmcheck.a         libpthread.a
crt1.o                libc.so.6               libmemusage.so      libpthread.so.0
crti.o                libc_malloc_debug.so    libmvec.a           libresolv.a
crtn.o                libc_malloc_debug.so.0  libmvec.so          libresolv.so
gconv                 libc_nonshared.a        libmvec.so.1        libresolv.so.2
gcrt1.o               libcrypt.a              libnsl.so.1         librt.a
ld-linux-x86-64.so.2  libcrypt.so             libnss_compat.so    librt.so.1
libBrokenLocale.a     libcrypt.so.1           libnss_compat.so.2  libthread_db.so
libBrokenLocale.so    libdl.a                 libnss_db.so        libthread_db.so.1
libBrokenLocale.so.1  libdl.so.2              libnss_db.so.2      libutil.a
libSegFault.so        libg.a                  libnss_dns.so.2     libutil.so.1
libanl.a              libm-2.34.a             libnss_files.so.2
libanl.so             libm.a                  libnss_hesiod.so
```

### 查看glibc的版本：

```bash
# 从上可知，ldd是glibc的核心组件之一
$ ldd --version
```

### 寻找libc.so的路径：

```bash
$ locate libc.so
/usr/lib/x86_64-linux-gnu/libc.so
/usr/lib/x86_64-linux-gnu/libc.so.6
$ locate libstdc++.so
/usr/lib/gcc/x86_64-linux-gnu/11/libstdc++.so
/usr/lib/x86_64-linux-gnu/libstdc++.so.6
/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.30
/usr/share/gdb/auto-load/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.30-gdb.py
```

### 安装glibc：

Ubuntu平台

```bash
sudo apt-get install lib6
```

RedHat平台

```bash
sudo yum install glibc
```

### 检查GNC C++ Library (libstdc++)的版本：

```bash
$ strings /usr/lib/libstdc++.so.* | grep LIBCXX
[sjcvl-zhigaoz ] /lan/cva_rel/vxe_main/24.02.650.d000/tools.lnx86/lib/64bit % strings /usr/lib/libstdc++.so.* | grep LIBCXX
GLIBCXX_3.4
GLIBCXX_3.4.1
GLIBCXX_3.4.2
...
GLIBCXX_3.4.19
GLIBCXX_DEBUG_MESSAGE_LENGTH

$ strings /usr/lib/libc.so.* | grep GLIBC
GLIBC_2.0
GLIBC_2.1
GLIBC_2.1.1
...
GLIBC_2.17
GLIBC_PRIVATE
```

如果你有一个使用了libstdc++的特定的binary或application，可以用下面的命令来检查其版本：

```bash
$ ldd <your_binary_or_application> | grep libstdc++
```

使用vscode的“Remote SSH”工具试图连接到Linux时，可能会报错如下：

> Warning: Missing GLIBCXX >= 3.4.25! from /usr/lib64/libstdc++.so.6.0.19
> Warning: Missing GLIBC >= 2.28! from /usr/lib64/libc-2.17.so
> Error: Missing required dependencies. Please refer to our FAQ https://aka.ms/vscode-remote/faq/old-linux for additional information.

这是因为Linux系统上的glibc版本中不包含GLIBCXX_3.4.25及以上的版本。此时需要降级vscode（建议做法）或升级glibc（似乎很难）。


### 让某些程序默认使用你本地安装的 glibc

你可以让某些程序默认使用你本地安装的 glibc（比如 ~/local/glibc-2.42），但不能全局替换系统默认 glibc，否则会导致系统崩溃。下面是几种安全又实用的方法，让你的程序“默认”使用本地 glibc：

✅ 方法一：使用 LD_LIBRARY_PATH 环境变量（推荐）
这是最简单的方式，让你运行的程序优先使用你指定的 glibc：

```bash
export LD_LIBRARY_PATH=$HOME/local/glibc-2.42/lib:$LD_LIBRARY_PATH
```

你可以把这行加到 ~/.bashrc 或 ~/.profile 中，这样每次登录都会自动生效。

📌 注意：这只影响你当前用户的 shell 环境，不会影响系统服务或其他用户。

✅ 方法二：使用 ld.so.preload（⚠️危险，不推荐）
你可以将本地 glibc 的路径写入 /etc/ld.so.preload，让系统优先加载它。但这会影响所有程序，包括 ls、bash、sudo 等，一旦路径错误，系统可能无法启动。

```bash
echo "$HOME/local/glibc-2.42/lib/libc.so.6" | sudo tee /etc/ld.so.preload
```
⚠️ 强烈不建议使用此方法，除非你在容器或 chroot 环境中测试。

✅ 方法三：使用 patchelf 修改程序的动态链接器
你可以用 patchelf 工具修改 ELF 程序的运行时 glibc 路径：

```bash
patchelf --set-interpreter $HOME/local/glibc-2.42/lib/ld-2.42.so \
         --set-rpath $HOME/local/glibc-2.42/lib \
         ./your_program
```

这样修改后的程序会自动使用你指定的 glibc，无需额外设置。

✅ 方法四：编译时指定 glibc 路径（适用于 CMake/GCC）
如果你自己编译程序，可以在编译时指定 glibc 的头文件和库路径：

```bash
gcc -I$HOME/local/glibc-2.42/include \
    -L$HOME/local/glibc-2.42/lib \
    -Wl,--rpath=$HOME/local/glibc-2.42/lib \
    your_code.c -o your_program
```

或者在 CMake 中设置：

```cmake
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-rpath=$HOME/local/glibc-2.42/lib")
```

🧠 总结：推荐做法

|           方法          | 是否推荐 | 说明             |
|:-----------------------:|----------|------------------|
| LD_LIBRARY_PATH         | ✅ 推荐   | 简单、安全、可控 |
| patchelf                | ✅ 推荐   | 精确控制某个程序 |
| 编译时指定路径          | ✅ 推荐   | 适合开发者       |
| 修改 /etc/ld.so.preload | ❌ 危险   | 可能导致系统崩溃 |


### 🔍 编译器如何找到 glibc
1. 默认系统路径
GCC 默认会在系统的标准库路径中查找 glibc，例如：

/lib
/lib64
/usr/lib
/usr/lib64
/usr/include（头文件）

这些路径通常由系统预设，glibc 安装时会自动放置在这些位置。

2. 头文件与动态库
编译阶段：GCC 会使用 /usr/include 下的 glibc 头文件（如 stdio.h, stdlib.h）来进行语法检查和类型推导。

链接阶段：GCC 会调用链接器（ld），查找 libc.so.6 或 libc.a 来完成符号解析。

3. 环境变量控制
你可以通过环境变量来影响 GCC 查找 glibc 的位置：

C_INCLUDE_PATH：指定头文件搜索路径

LIBRARY_PATH：指定库文件搜索路径

LD_LIBRARY_PATH：运行时动态库搜索路径

例如：

bash
export C_INCLUDE_PATH=/opt/glibc/include
export LIBRARY_PATH=/opt/glibc/lib
4. 链接器参数
你也可以通过 GCC 的 -Wl 参数直接告诉链接器使用哪个 glibc：

bash
gcc hello.c -o hello \
  -Wl,--dynamic-linker=/opt/glibc/lib/ld-2.34.so \
  -L/opt/glibc/lib
这会指定使用 /opt/glibc/lib/libc.so.6 和对应的动态链接器。

5. 使用 patchelf 工具修改依赖
如果你已经编译好了程序，但想修改它使用的 glibc，可以用 patchelf：

bash
patchelf --set-interpreter /opt/glibc/lib/ld-2.34.so hello
patchelf --replace-needed libc.so.6 /opt/glibc/lib/libc.so.6 hello
这在部署多版本 glibc 时非常有用。

🧪 如何验证 glibc 的使用情况
查看程序依赖的 glibc：

bash
ldd ./hello
查看系统 glibc 版本：

bash
ldd