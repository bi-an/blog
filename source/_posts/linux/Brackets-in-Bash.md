---
title: Brackets in Bash
date: 2024-01-25 13:22:53
categories: Linux
tags: shell
---

## 中括号

1. `[ ]`和`test`是bash的内部命令，`[[ ]]`是shell的条件判断关键字。

    ```bash
    $ type [
    [ is a shell builtin
    $ type test
    test is a shell builtin
    $ type [[
    [[ is a shell keyword
    ```

2. `[ ]`和`test`是等价的，用于评估条件表达式。可以使用`man [`或`help [`查阅帮助文档。

    ```bash
    $ help [
    [: [ arg... ]
        Evaluate conditional expression.
        
        This is a synonym for the "test" builtin, but the last argument must
        be a literal `]', to match the opening `['.
    ```

3. `[[ ]]`关键字可以屏蔽shell特殊符号，比如`&&`、`||`、`>`和`<`可以被认为是条件判断符而不是重定向符。
4. `[ ]`中使用`-a`和`-o`表示逻辑与和逻辑或，`[[ ]]`中则使用`&&`和`||`。

## 小括号

1. `$()`用于命令替换。
2. 双小括号`(( ))`：在比较过程中使用高级数学表达式。

## 大括号

请阅读：[All about {Curly Braces} in Bash](https://www.linux.com/topic/desktop/all-about-curly-braces-bash/)

1. `${}`用于引用变量。

    与`$var`相比，`${var}`是一种消除歧义的措施，比如：

    ```bash
    $ var=abc
    $ vartest=ABC
    # $var引用变量'var'
    $ echo $var
    abc
    # 引用变量'vartest'
    $ echo $vartest
    ABC
    # 引用变量'var'并在其后加上'test'字符
    $ echo ${var}test
    abctest
    ```

2. `{}`表示分组。



## Reference

* [Shell中test、单中括号、双中括号的区别](https://www.cnblogs.com/zeweiwu/p/5485711.html)
* [Differences Between Single and Double Brackets in Bash](https://www.baeldung.com/linux/bash-single-vs-double-brackets)
* [Shell中的括号、双括号、方括号和双方括号](https://www.jianshu.com/p/3e1eaaa3fee8)
