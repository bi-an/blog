---
title: Bash Debugging
date: 2024-09-06 18:10:12
categories: Linux
tags: shell
---

## 启用调试模式

1. 命令行启用调试：

```bash
#使用 -x 和 -v 选项运行 main.sh 脚本。
bash -xv main.sh
```

注意：调试输出是通过 `stderr` ，在试图重定向时要注意。

2. 在脚本中启用调试：

```bash
set -xv
```

| Short notation | Long notation  | Result                                                         |
|----------------|----------------|----------------------------------------------------------------|
| set -f         | set -o noglob  | Disable file name generation using metacharacters (globbing).  |
| set -v         | set -o verbose | Prints shell input lines as they are read.                     |
| set -x         | set -o xtrace  | Print command traces before executing command.                 |

参考[链接](https://tldp.org/LDP/Bash-Beginners-Guide/html/sect_02_03.html)

其他选项：

- `set +e`：忽略错误继续运行（bash）。
- `set -e`：遇错停止执行（bash）。

3. 环境变量 `PS4`

```bash
PS4='+ ${LINENO}: '
```

可以搭配使用 `PS4` 环境变量，以在调试输出中打印命令所在的文件行号。

`PS4` 是一个 Bash 环境变量，用于定义调试模式下的提示符格式。
当你启用调试模式（通过 `set -x`）时，Bash 会在每一行执行之前打印调试信息，而 PS4 决定了这些调试信息的格式。
`$LINENO`表示打印行号。


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