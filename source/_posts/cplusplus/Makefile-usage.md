---
title: Makefile usage
categories: c/cpp
tags: build
date: 2023-11-17 14:19:35
---

## "make run" argument

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

## 括号

引用变量时，Shell使用大括号，Makefile则大括号和小括号都行。但是在命令中使用Shell变量就需要使用大括号。

[参考](https://blog.csdn.net/bigmarco/article/details/6687337)