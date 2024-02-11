---
title: C++ Initializer List
date: 2024-01-26 10:31:32
categories: c/cpp
tags: syntax
---

## 列表初始化

struct/union/array默认支持列表初始化。

[Struct and union initialization](https://en.cppreference.com/w/c/language/struct_initialization)
[Array initialization](https://en.cppreference.com/w/c/language/array_initialization)

```cpp
#include <bits/stdc++.h>
// #include <initializer_list>
using namespace std;

class A {
public:
    int x, y, z; // 注意：数据成员必须是public的。
    void print() {
        cout << x << " " << y << " " << z << endl;
    }
};

class B {
public:
    vector<int> vec_;
    int x_;
    void print() {
        cout << "vec_={ ";
        for (int x : vec_) {
            cout << x << " ";
        }
        cout << "}, x_=" << x_ << endl;
    }
};

class C {
public:
    vector<int> vec_;
    void print() {
        cout << "vec_={ ";
        for (int x : vec_) {
            cout << x << " ";
        }
        cout << "}" << endl;
    }
};

int main() {
    // C++会构造一个列表初始化的默认构造函数，
    // 以下(1)和(2)都是调用这个默认构造函数。
    A a1{1,2,3}; // (1) 列表初始化
    A a2({4,5,6}); // (2) 同(1)
    a1.print();
    a2.print();

    B b1{{1,2,3}, 4}; // 内部的"{1,2,3}"用于构造vec_，"4"用于初始化x_
    b1.print();

    // C C1{1,2,3}; // (1) 这里会报错"error: too many initializers for ‘C’"，
    //              //     因为C只有一个数据成员vec_，这里却传入了3个参数
    C c2{{1,2,3}}; // (2) 其中，内部的"{1,2,3}"用于构造vec_，外层的"{}"用于对c2本身进行构造
    c2.print();

    return 0;
}
```

## std::initializer_list

```cpp
#include <bits/stdc++.h>
#include <initializer_list>
using namespace std;

struct A {
    int x, y, z;

    A(initializer_list<int> il) {
        initializer_list<int>::iterator it = il.begin();
        x = *it++;
        y = *it++;
        z = *it++;
    }

    void print() {
        cout << x << " " << y << " " << z << endl;
    }
};

struct B {
    vector<int> vec;

    B(initializer_list<int> il) {
        vec = il; // 用initializer_list初始化vector
    }

    void print() {
        cout << "{ ";
        for (int x : vec) {
            cout << x << " ";
        }
        cout << "}" << endl;
    }
};

int main() {
    A a{1,2,3};
    a.print();

    B b{4,5,6};
    b.print();

    return 0;
}
```