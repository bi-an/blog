---
title: gcc Usage
date: 2024-01-23 21:44:11
categories: c/cpp
tags: gcc
---

## 递归处理顺序

gcc 的输入文件和库是从左往右处理的。也就是说，以下命令是错误的：

```bash
gcc -L. -la main.cc
```

链接器处理到某个目标文件（如 main.cc 编译后的目标代码）时，如果遇到未解析的符号（比如 f() ），
它会从接下来的库中查找这些符号。因此顺序非常重要。

这里，-L. -la 选项在 main.cc 之前，链接器会首先尝试从 liba.so 中查找引用的符号，
但是，因为此时 main.cc 还未被处理，所以链接器还不知道有对 liba.so 中的函数 f() 的引用。
到了 main.cc ，链接器解析出引用，但它不会回头再去 liba.so 中查找，导致报错："undefined reference to f()"。

正确的命令如下：

```bash
gcc main.cc -L. -la
```

注意：liba.so 的指定必须去掉 lib 和 .so ，也就是说不允许直接指定“库文件名”，而是只能指定“库名”。
如果想直接指定库文件名，那么应该把 liba.so 当成输入文件：

```bash
gcc main.cc ./liba.so
```

## 属性语法（Attribute Syntax）

参考：[官方文档](https://gcc.gnu.org/onlinedocs/gcc-3.2/gcc/Attribute-Syntax.html#Attribute%20Syntax)。

### 函数属性（Function Attributes）

参考：

* [stackoverflow](https://stackoverflow.com/questions/11621043/how-should-i-properly-use-attribute-format-printf-x-y-inside-a-class)
* [gcc官方文档：Function Attributes](https://gcc.gnu.org/onlinedocs/gcc-3.2/gcc/Function-Attributes.html)

属性列举：

* `format (archetype, string-index, first-to-check)`

    format 属性，指定函数采用 printf、scanf、strftime 或 strfmon 风格的参数，
    这些参数应根据格式字符串（format string）进行类型检查（type-checked）。
    类型检查发生在编译期。

    举例：
    ```cpp
    extern int
    my_printf (void *my_object, const char *my_format, ...)
        __attribute__ ((format (printf, 2, 3)));
    ```

  - `archetype`决定format string应该如何解释。
  可选为`printf`、`scanf`、`strftime`或`strfmon`（也可以使用`__printf__`、`__scanf__`、`__strftime__`或`__strfmon__`）。
  - `string-index`指定哪个参数是format string（从1开始）。
  - `first-to-check`指定format string对应的第一个参数的序号。
  对于那些无法检查参数的函数（比如`vprintf`），该参数指定为`0`。在这种情况下，编译器仅检查format string的一致性。对于`strftime`格式，该参数必须为`0`。


## 选项

* `-save-temps`: 可以保留所有中间文件，例如预编译文件、汇编文件、目标文件等。

