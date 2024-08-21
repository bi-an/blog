---
title: gcc Usage
date: 2024-01-23 21:44:11
categories: c/cpp
tags: gcc
---

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
