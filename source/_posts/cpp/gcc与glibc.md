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
