---
title: Cpp Defects
date: 2024-01-23 21:33:51
categories: c/cpp
tags: debug
---

## basic_string::_M_construct null not valid

C++有一个缺陷，请看以下代码：

```cpp
//cpp defeat: basic_string::_M_construct null not valid
void fun(string s) {
    cout << "fun" << endl;
}

int main() {
    fun(0); // Run-time error: basic_string::_M_construct null not valid
}
```

其中，`fun(0)`的`0`会被视为`const char*`类型，也就是`nullptr`，所以在编译期可以通过。
但是运行期会触发`string`对象的构造错误“basic_string::_M_construct null not valid”。

隐蔽一点的代码：

```cpp
char * get_a_string() {
    return nullptr;
}

int main() {
    // Attention: Alaways take care that a parameter to a string should not be NULL!
    fun(get_a_string()); // Run-time error: basic_string::_M_construct null not valid
    // Better code
    char * str = get_a_string();
    fun(str != NULL? str : "");
}
```