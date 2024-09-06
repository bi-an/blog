---
title: Bash Debugging
date: 2024-09-06 18:10:12
categories: Linux
tags: shell
---

# bash调试技巧

## `set -x`

```bash
PS4='+ ${LINENO}: '
set -x
```

说明：`PS4`设置打印格式，`$LINENO`表示打印行号。

## `trap DEBUG`

设置DEBUG陷阱，该陷阱在每一条“简单语句”执行完毕之后会被调用一次。

```bash
set -o functrace
track_var="my_var"
old_value=""
check_var_change() {
    new_value="${!track_var}"
    if [[ "$new_value" != "$old_value" ]]; then
        echo "$track_var changed to \"$new_value\" at $BASH_LINENO"
        old_value="$new_value"
    fi
}
trap check_var_change DEBUG
```

说明：监控`my_var`变量的变化，如果变化，则打印新值。

注意：实际测试中，我们发现`trap`监控变量变化似乎会延迟，而`set -x`打印的信息是可靠的。

## 特殊变量

参考：https://www.gnu.org/software/bash/manual/html_node/Bash-Variables.html

- `BASH_LINENO`: 这是一个数组变量，‌用于表示当前执行的脚本或函数中每一行的行号。‌它特别适用于调试，‌因为它可以追踪到函数调用栈中每一层的行号。‌通过访问BASH_LINENO数组的不同索引，‌可以获取到不同函数调用层级对应的行号信息‌。
- `LINENO`: 获取当前脚本行号‌，‌而不涉及函数调用栈的追踪‌。
- `FUNCNAME`: 函数名
- `BASH_SOURCE`