---
title: C++右值引用
date: 2025-08-16 10:54:08
categories:
tags: c/cpp
---


## 1. 左值、将亡值、纯右值

C++11的值必定属于：左值、右值（将亡值、纯右值）三者之一。不是左值就是右值。详见值类别。

- **左值**的特点：“有名字、可以取址”。没有名字或者不能取址，则必定是右值。
- **右值**的特点：即将消亡，也就是说“会被析构”。
  - **纯右值**：一定没有名字。比如除去`string`之外字面值常量、函数返回值、运算表达式。
  - **将亡值**：即将消亡的值：比如临时变量，一旦离开作用域就会被销毁；可能没有名字，例如函数的返回值（非引用）。

示例：

```cpp
int main() {
    A(); // 匿名对象的作用域仅限于语句中，一旦离开当前语句，就会析构。
    getchar(); // 暂停
    return 0;
}
```

---

## 2. 引用、右值引用

右值引用涉及“右值”和“引用”两个概念。

- **引用**不是对象，所以定义一个“右值引用”不会调用构造函数，避免了多余的构造过程。
- **右值**是即将析构的值，把右值绑定到右值引用上，延长了右值的生命期，所以右值对象没有析构。

### 右值引用规则：

- 可以把左值绑定到左值引用。
- 可以把右值绑定到右值引用。
- 不允许把左值绑定到右值引用。
- 不允许把右值绑定到左值引用。
- `const`左值引用可以接受左值或右值。

示例：

```cpp
int a1 = 10; // 10是纯右值
const int& aa = 10; // 常量引用可以接受右值
int&& aaa = 10; // 右值引用接受右值
```

---

## 3. 引用和右值引用是左值

引用（包括右值引用）本身是左值，可以取址，但不能对右值取址。

示例：

```cpp
#include <utility>
using namespace std;

void fun1(int& t) { // 接受一个左值参数
}

void fun2(int&& t) { // 接受一个右值参数，但t本身是左值
}

int main() {
    int a = 10;
    int&& ra = move(a); // move(a)返回一个右值，ra却是一个左值

    fun1(ra); // 正确：ra是左值，可以绑定到左值引用
    fun2(move(a)); // 正确：move(a)返回一个右值

    return 0;
}
```

---

## 4. 复制构造函数和移动构造函数

### 为什么右值引用的构造函数被视为“移动”语义？

因为输入参数是一个引用（右值引用也是引用），可以直接访问所引对象的资源并接管它，同时将源对象的资源置空。

示例：

```cpp
#include <iostream>
using namespace std;

class A {
public:
    A() { cout << "A()" << endl; }
    A(const A&) { cout << "A(const A&)" << endl; }
    A(A&&) { cout << "A(A&&)" << endl; }
};

int main() {
    A a;
    A b(std::move(a)); // 调用移动构造函数
    return 0;
}
```

---

## 5. 完美转发

完美转发是指对模板参数实现完美转发：即输入什么类型（左值、右值）的参数，就是什么类型的参数。

### 引用折叠规则：

- 如果有左值引用，优先折叠成左值引用。
- 如果只有右值引用，参数推导成右值引用。

示例：

```cpp
#include <iostream>
using namespace std;

void RunCode(int& m) { cout << "lvalue ref" << endl; }
void RunCode(int&& m) { cout << "rvalue ref" << endl; }

template<typename T>
void PerfectForward(T&& t) {
    RunCode(static_cast<T&&>(t)); // 保证完美转发
}

int main() {
    int a = 10;
    PerfectForward(a); // lvalue ref
    PerfectForward(move(a)); // rvalue ref
    return 0;
}
```

---

## 6. 移动构造函数、移动赋值函数、复制构造函数、复制赋值函数

### 移动构造函数的注意事项：

- 移动构造函数不允许抛出异常，建议添加`noexcept`关键字。
- 使用`std::move_if_noexcept`可以在移动构造函数抛出异常时回退到复制构造函数。

示例：

```cpp
#include <iostream>
using namespace std;

class A {
public:
    A() { cout << "A()" << endl; }
    A(const A&) { cout << "A(const A&)" << endl; }
    A(A&&) noexcept { cout << "A(A&&)" << endl; }
};

int main() {
    A a;
    A b(std::move(a)); // 调用移动构造函数
    return 0;
}
```

---

## 7. 编译器优化

编译器默认会采用“返回值优化”（RVO或NRVO）。要观察移动语义与复制语义的不同，应该关闭编译器优化。

关闭优化命令：

```bash
g++ -o test main.cc -fno-elide-constructors
```

---

## 8. 合成的移动操作

如果没有定义复制构造/赋值函数，编译器会为我们合成（浅复制）。但如果自定义了复制构造函数、复制赋值运算符或析构函数，编译器将不会合成移动构造/赋值函数。

示例：

```cpp
#include <iostream>
using namespace std;

class A {
public:
    A() { cout << "A()" << endl; }
    A(const A&) { cout << "A(const A&)" << endl; }
    A(A&&) { cout << "A(A&&)" << endl; }
};

int main() {
    A a;
    A b(std::move(a)); // 调用移动构造函数
    return 0;
}
```

---

## 9. `std::move`的实现

`std::move`是一个类型转换，没有完成其他工作。

实现：

```cpp
template<class T>
constexpr typename std::remove_reference<T>::type&& move(T&& t) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}
```

---

## 10. `unique_ptr`与`std::move`

示例：

```cpp
#include <iostream>
#include <memory>
using namespace std;

int main() {
    unique_ptr<int> up(new int(10));
    unique_ptr<int> p = std::move(up); // 现在up为空
    return 0;
}
```

解释：`std::move`调用`unique_ptr`的移动构造函数，转移`up`所拥有的资源，并将`up`置为空。

---

## 11. 右值与`sizeof`

示例：

```cpp
struct A {
    int x, y;
};

int f() {
    return 1;
}

int main() {
    cout << sizeof(int()) << endl; // 1
    cout << sizeof(10) << endl; // 4
    cout << sizeof(A) << endl; // 8
    cout << sizeof(A()) << endl; // 1
    cout << sizeof(f()) << endl; // 4
}
```

---

## 参考资料

- 《深入理解C++11: C++新特性解析与应用》
- 《C++ Primer 第五版》
- C++中文 - API参考文档

