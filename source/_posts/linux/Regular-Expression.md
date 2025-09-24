---
title: Regular Expression
date: 2024-01-25 13:13:41
categories: Linux
tags: shell
---

## regexp

## ERegExp

## Wildcard/Glob

`man 7 glob`

> glob - globbing pathnames. glob is a shell built-in.
> 
> 主要用于匹配带有通配符的文件路径。其匹配字符串的能力比正则表达式弱。
> 
> 它最初是贝尔实验室 Unix 系统上的一个名叫 glob 的命令（glob 是 global 的缩写），用于展开命令行中的通配符。后来系统提供了该功能的 C 语言库函数glob()，知名的 shell 解释器就使用了该接口，shell 脚本和命令行中使用的 glob 模式匹配功能便源自于此。——见[博客](https://juejin.cn/post/6844904077801816077)。

[Wildcards](https://tldp.org/LDP/GNU-Linux-Tools-Summary/html/x11655.htm)

`{}`严格来讲不属于glob的范畴，其在shell表示一个分组，见：[All about {Curly Braces} in Bash](https://www.linux.com/topic/desktop/all-about-curly-braces-bash/)。

## sed

## awk

[awk Command](https://www.ibm.com/docs/nl/aix/7.2?topic=awk-command)

### 格式

```bash
awk -F' *|:' '/LISTEN/{print $2}'
```

其中，`-F`表示分隔符；
` *|:`是一个正则表达式，表示以"一个或多个空格"或":"作为分隔符；
再其后的`//`中是另一个正则表达式，用于匹配文本；
`{}`中是action。

### 条件判断

[if-else in awk](https://tecadmin.net/awk-conditional-statements/)
[use AND and OR in an awk program](https://unix.stackexchange.com/questions/660178/how-to-use-and-and-or-together-in-an-awk-program)

```bash
awk '{if ($1 > 49151 && $1 < 65536) {print $1} }'
```

等价于
```bash
awk '$1 > 49151 && $1 < 65536'
```

### BEGIN/END

In AWK, `BEGIN` and `END` are special patterns that allow you to execute code before processing the input (`BEGIN`) or after processing all the input (`END`).

- The `BEGIN` block is executed once at the beginning of the AWK program and it is typically used for initializing variables or performing setup tasks.
- The `END` block is executed once after processing all input, and it is commonly used for final caclucations, summaries, or printing results.

### Special symbols

- `$` 用于引用field，例如`$1`代表第一个field（典型来说是第一列）。
- `NF` 表示number of filed，假设一共有7列，那么`$NF`与`$7`等价。

## grep

## Example

1. 找出一个未使用的port

{% raw %}
{% tabs Code %}

<!-- tab Makefile -->

```makefile
# "$$"是为了转义"$"
max_port=$(shell netstat -antulpen 2> /dev/null \
    | awk -F' *' '/^(tcp|udp)/{print $$4}' | cut -d: -f 2 \
    | egrep "\w" | sort | tail -1)
#$(warning ${max_port})
port=$(shell expr $(max_port) + 1)
#$(warning ${port})
```

<!-- endtab -->

<!-- tab Bash -->

```bash
# 从1025开始找出已经存在的端口，如果相邻端口的gap大于1，则返回“当前端口号+1”
netstat -ant | awk '{print $4}' \
    | awk -F: '{if ($NF ~ /^[0-9]+$/ && $NF > 1024) {print $NF}}' \
    | awk 'BEGIN {prev = 1024} 
        {if ($1 - prev > 1) { port = prev + 1; exit} else { prev = $1}} 
        END { if (port=="") {print prev + 1} else {print port}}'
```

<!-- endtab -->

<!-- tab Python-->

```python
#这不是正则表达式
#以0端口为参数创建一个socket，则系统会自动选择一个未使用的端口号

#one-line mode:
#python -c 'import socket; s=socket.socket(); s.bind(("", 0)); print(s.getsockname()[1]); s.close()'
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('', 0))
addr = s.getsockname()
print(addr[1])
s.close()
```

<!-- endtab -->

{% endtabs %}

{% endraw %}
