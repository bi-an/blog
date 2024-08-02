---
title: libtirpc Usage
date: 2024-07-08 14:57:11
tags: RPC
---

## Links

- [TI-RPC](https://www.ibm.com/docs/en/i/7.3?topic=communications-using-sun-ti-rpc-develop-distributed-applications)
- [RPC language](https://www.ibm.com/docs/en/aix/7.1?topic=call-rpc-language)
- [XDR Protocol](https://docs.oracle.com/cd/E19683-01/816-1435/xdrproto-61652/index.html)
- [PRC Compiler](https://www.ibm.com/docs/en/i/7.3?topic=applications-using-rpcgen-compiler)
- [rpcgen](https://docs.oracle.com/cd/E19683-01/816-1435/rpcgenpguide-21470/index.html)
- [rpcgen programming guide](https://docs-archive.freebsd.org/44doc/psd/22.rpcgen/paper.pdf)

```bash
$ which rpcgen
/bin/rpcgen
$ rpcgen
usage: rpcgen infile
        rpcgen [-abkCLNTM][-Dname[=value]] [-i size] [-I [-K seconds]] [-Y path] infile
        rpcgen [-c | -h | -l | -m | -t | -Sc | -Ss | -Sm] [-o outfile] [infile]
        rpcgen [-s nettype]* [-o outfile] [infile]
        rpcgen [-n netid]* [-o outfile] [infile]
options:
-a              generate all files, including samples
-b              backward compatibility mode (generates code for SunOS 4.1)
-c              generate XDR routines
-C              ANSI C mode
-Dname[=value]  define a symbol (same as #define)
-h              generate header file
-i size         size at which to start generating inline code
-I              generate code for inetd support in server (for SunOS 4.1)
-K seconds      server exits after K seconds of inactivity
-l              generate client side stubs
-L              server errors will be printed to syslog
-m              generate server side stubs
-M              generate MT-safe code
-n netid        generate server code that supports named netid
-N              supports multiple arguments and call-by-value
-o outfile      name of the output file
-s nettype      generate server code that supports named nettype
-Sc             generate sample client code that uses remote procedures
-Ss             generate sample server code that defines remote procedures
-Sm             generate makefile template 
-t              generate RPC dispatch table
-T              generate code to support RPC dispatch tables
-Y path         directory name to find C preprocessor (cpp)

For bug reporting instructions, please see:
<http://www.gnu.org/software/libc/bugs.html>.
```

Note:


`%` 可以用来转义某行，任何句首带有`%`的行将被视作字符串，直接放入到输出文件中。
注意：`rpcgen`可能改变该行的位置，所以应该在输出文件中对这些行进行仔细检查。

`rpcgen` provides an additional preprocessing feature: any line that begins with a percent sign (`%`) is passed directly to the output file, with no action on the line's content. Use caution because `rpcgen` does not always place the lines where you intend. Check the output source file and, if needed, edit it.

例如

```rpc
#include "abc.h"
```

当执行`rpcgen infile`命令时，由于"abc.h"不存在，可能会报错。

但是如果在句首添加一个`%`符号，则可以绕过检查。

```rpc
%#include "abc.h"
```