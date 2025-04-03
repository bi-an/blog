---
title: Makefile入门
categories: c/cpp
tags: build
date: 2023-11-17 14:19:35
---

## Makefile默认shell

Makefile 默认的 shell 是 /bin/sh:
查看 Makefile 默认的 shell:
$(warning ${SHELL}) # 查看默认 shell

如果在 Makefile 开头加上以下语句，可以指定 shell:
SHELL := /bin/bash

Makefile 的 shell 命令似乎每一条都会新开辟一个 shell 环境来执行，因为每条 shell 命令似乎都执行了一次 .cshrc 脚本。
要想在同一个 shell 环境中执行所有命令，则需要使用分号分割并转义换行符。

## Makefile环境变量

你可以通过设置环境变量 MAKEFLAGS 来让所有的子shell都将 make 认作 make -n：
export MAKEFLAGS="-n"
注：-n, --dry-run 表示只打印将要执行的命令，但是不真正执行它们。


Makefile 设置 PATH 环境变量：
PATH := mypath:$(PATH)
必须使用 ":=" ，因为 ":=" 是简单赋值，但是 "=" 会递归展开，会导致报错 “Recursive variable `PATH' references itself (eventually)”。

Makefile 定义多行变量：使用 define 语句。
Makefile 接收命令行参数

非常奇特的现象：
$ make clean
cat clean.sh >clean 
chmod a+x clean
因为 Makefile 文件中没有 clean 的 recipe，但是当前目录下有个 clean.sh 文件。
但是，当再次执行 make clean，clean 脚本还是不会被执行：
$ make clean
make: `clean' is up to date.

## 特殊符号

### 空格

Makefile对空格的处理，似乎是：从第一个非空格开始，到明确的截止符（换行、逗号、括号、注释标记'#'等）为止。
a =   b   #注意末尾有3个空格
$(warning a=$(a)c)

结果：
a=    b   c

Makefile转义符：
字符	转义方法
$	$$
#	\#
\	\

注意：这里说的是Makefile中的转义符，不是bash中的转义符。

### 括号

引用变量时，Shell使用大括号，Makefile则大括号和小括号都行。但是在命令中使用Shell变量就需要使用大括号。

[参考](https://blog.csdn.net/bigmarco/article/details/6687337)

## Makefile调试方法

要调试 Makefile 并查看其执行过程，可以使用以下几种方法：
	1. 使用 make 的 -n 或 --dry-run 选项：这将显示 Makefile 中的所有命令，而不会真正执行它们。
make -n
	1. 使用 make 的 -d 或 --debug 选项：这将显示详细的调试信息，包括变量的展开和规则的匹配过程。
make -d
	1. 使用 make 的 -p 或 --print-data-base 选项：这将打印所有的变量、规则和隐含规则。
make -p
	1. 在 Makefile 中添加调试信息：你可以在 Makefile 中添加一些调试信息，例如使用 $(info ...) 来打印变量的值。
print:
    @$(foreach V, $(.VARIABLES), $(info $(V) = $($(V))))
	1. 使用 make 的 -j 选项：如果你使用并行执行，可以使用 -j 选项来限制并行任务的数量，并更容易地跟踪输出。
make -j1

makefile打印所有变量的值：
debug:
    @$(foreach V, $(.VARIABLES), $(info $(V) = $($(V))))
然后在命令行中运行：
make debug
这将打印所有变量及其值。


print-%:
    @echo $* = $($*)

print-% 是一个 Makefile 目标，用于打印变量的值。具体步骤如下：
	1. 定义一个目标 print-%，其中 % 是一个通配符，表示任意变量名。
	2. 使用 @echo $* = $($*) 打印变量名和变量值。

示例代码：
print-%:
    @echo $* = $($*)
使用方法：
在命令行中运行：
make print-VARIABLE_NAME
例如：
make print-XTENSA_SW_RELEASE
这将打印 XTENSA_SW_RELEASE 变量的值。

## 命令行参数

See [here](https://stackoverflow.com/questions/2214575/passing-arguments-to-make-run).

`@:` See [here](https://unix.stackexchange.com/questions/92978/what-does-this-2-mean-in-shell-scripting).

This works fine for me:

```makefile
# If the first argument is "run"...
ifeq (run,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "run"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  # TODO: What does the following line mean?
  # $(eval $(RUN_ARGS):;@:)
endif

# "cmd" refers to any command
run:
	cmd $(RUN_ARGS)
```

## makefile同名目标处理方式

参考：

* [链接](https://blog.csdn.net/lixiangminghate/article/details/50448664)
* [链接](https://stackoverflow.com/questions/43718595/two-targets-with-the-same-name-in-a-makefile)

[makefile将命令结果赋值给变量](https://stackoverflow.com/questions/2019989/how-to-assign-the-output-of-a-command-to-a-makefile-variable)

## Makefile中短划线

```makefile
all:
	-/bin/rm -rf *.log
```

其中，"`-/bin/rm`"的短划线"`-`"是一个特殊前缀，表示忽略命令执行过程的错误。

## 为每个源文件生成一个可执行程序

```makefile
SRCS = $(wildcard *.c)

all: $(SRCS:.c=)

# Unnecessary, as the default rules are adequate.
.c:
	gcc $(CPFLAGS) $< -o $@
```

最后两行其实不需要，默认规则已经足够了。

其中，`$(SRCS:.c=.o)`表示将变量`SRCS`中的每个单词（以空格分割）中的`.c`替换为`.o`。以上代码则是将所有`.c`都去掉。
