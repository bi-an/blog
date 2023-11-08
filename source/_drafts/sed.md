---
title: Linux命令sed
tags:
---
sed，a stream editor，参考[GNU文档](https://www.gnu.org/software/sed/manual/)。

## 正则表达式

基本正则表达式和扩展正则表达式的区别：

[5.2 Basic (BRE) and extended (ERE) regular expression](https://www.gnu.org/software/sed/manual/html_node/BRE-vs-ERE.html#BRE-vs-ERE)

基本正则表达式和扩展正则表达式是指定模式语法的两种变体。在sed中，这两种变体的区别只在少数特殊字符的行为上：
'`?`', '`+`', 小括号'`()`', 花括号'`{}`' 和 '`|`'。

基本正则表达式中，这些特殊字符没有特殊语义，除非前缀有 '`\`' 反斜杠；
扩展正则表达式则相反，这些特殊字符具备特殊语义，除非被 '`\`' 转义。

例子：

``` bash
$ echo "Welcome To The Geek Stuff" | sed 's/\(\b[A-Z]\)/\(\1\)/g'
```

## 参考资料

* [The Linux Documentation Project](https://tldp.org/LDP/abs/html/x23170.html)
* [gnu](https://www.gnu.org/software/sed/manual/html_node/Regular-Expressions.html)
* [geeksforgeeks](https://www.geeksforgeeks.org/sed-command-in-linux-unix-with-examples/)
