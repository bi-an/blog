---
title: C++ 弱符号（Weak Symbol）的定义与使用
date: 2026-10-20 12:20:00
categories: cpp
tags:
 - cpp
 - compile-link
---

在 C++（以及 C）中，弱符号（Weak Symbol）是链接器（Linker）的一个概念。它与强符号（Strong Symbol）相对，主要用于控制多个目标文件链接时的符号冲突解决策略，以及实现可选的插件 / 回调机制。

<!-- more -->

## 一、弱符号的定义

在链接过程中，符号分为强符号和弱符号：

- **强符号**：通常指未初始化的全局变量、已初始化的全局变量、已定义的函数（注：GCC 10 起默认 `-fno-common`，未初始化全局变量也是强符号；更早版本会放入 COMMON 段，行为类似弱符号）。
- **弱符号**：通过编译器扩展（如 GCC / Clang 的 `__attribute__((weak))`）显式声明的符号。

ELF 符号表中，符号的**绑定（Bind）**有三种：`LOCAL`、`GLOBAL`、`WEAK`。其中 `GLOBAL` 就是通常所说的“强符号”，`WEAK` 是“弱符号”。`nm` 输出里分别对应：

| `nm` 标记 | 含义 |
|-----------|------|
| `T` / `D` / `B` | 强（全局）定义，分别在 text / data / bss |
| `W` | 弱**定义**（`V` 在部分平台表示弱定义的对象） |
| `w` | 弱**未定义**（弱引用） |
| `U` | 强未定义（普通外部引用） |

用 `readelf` 看更直观，`Bind` 一列为 `WEAK`：

{% include_code lang:cpp tech/cpp/compile-link/weak-symbol-01.cpp %}

```bash
$ g++ -c weak-symbol-01.cpp
$ readelf -sW weak-symbol-01.o | grep -E "Bind|on_event"
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     4: 0000000000000000    26 FUNC    WEAK   DEFAULT    1 on_event
```

### 链接规则（符号决议）

1. **同名强符号冲突**：如果多个目标文件中存在同名的强符号，链接器会报错（`multiple definition`）。
2. **强弱符号并存**：如果同时存在一个强符号和一个或多个弱符号，链接器优先选择强符号，忽略弱符号。
3. **仅有弱符号**：如果只有弱符号，链接器会使用该弱符号（多个弱符号时，GNU ld 通常选链接顺序中靠前的，但不应依赖该行为）。
4. **未定义的弱引用（Weak Reference）**：如果一个弱符号没有被定义（即找不到任何强符号或弱符号定义），链接器不会报错，而是将其地址解析为 `nullptr`（零）。

## 二、生成弱符号的源码示例

在 GCC 和 Clang 中，可以通过 `__attribute__((weak))` 来定义弱符号或声明弱引用。

### 示例 1：弱定义（提供默认实现，允许被覆盖）

假设有一个基础库，里面有一个默认功能，但允许用户在主程序中重写它。

库文件 `library.cpp`（包含弱符号定义）：

{% include_code library.cpp lang:cpp tech/cpp/compile-link/weak-symbol-02.cpp %}

不重写弱符号的主程序 `main_default.cpp`：

{% include_code main_default.cpp lang:cpp tech/cpp/compile-link/weak-symbol-03.cpp %}

通过强符号重写弱符号的主程序 `main_override.cpp`：

{% include_code main_override.cpp lang:cpp tech/cpp/compile-link/weak-symbol-04.cpp %}

编译运行：

```bash
# 不重写：使用库中的弱定义
$ g++ library.cpp main_default.cpp -o app_default && ./app_default
[Library] Executing task...
[Library] Default weak hook implementation.

# 重写：main_override.cpp 中的强符号覆盖弱符号
$ g++ library.cpp main_override.cpp -o app_override && ./app_override
[Library] Executing task...
[Main] Overridden strong hook implementation!
```

整个过程没有任何链接错误，覆盖发生在链接阶段，`execute_task()` 的代码一个字节都没改。

用 `nm` 可以确认库中的符号是弱定义（`W`）：

```bash
$ g++ -c library.cpp -o library.o && nm library.o | grep hook
0000000000000000 W user_custom_hook
```

> 这里使用 `extern "C"` 是为了避免 C++ name mangling，使强弱符号名称一一对应，便于覆盖。

### 示例 2：弱引用（检测可选模块是否存在）

弱引用常用于插件化或可选依赖的代码：如果某个函数没有被链接进来，程序不会链接失败，而是在运行时做安全检查。

`plugin_checker.cpp`：

{% include_code plugin_checker.cpp lang:cpp tech/cpp/compile-link/weak-symbol-05.cpp %}

不链接提供 `optional_plugin_init` 的文件，程序依然可以正常编译运行，并安全跳过插件初始化：

```bash
$ g++ plugin_checker.cpp -o checker && ./checker
Checking for optional plugin...
Plugin not found. Skipping optional feature.
```

若同时链接提供强定义的文件（`plugin_impl.cpp`），插件自动生效：

{% include_code plugin_impl.cpp lang:cpp tech/cpp/compile-link/weak-symbol-06.cpp %}

```bash
$ g++ plugin_checker.cpp plugin_impl.cpp -o checker_plugin && ./checker_plugin
Checking for optional plugin...
Plugin found! Initializing...
[Plugin] initialized.
```

### 示例 3：C++ 中天然的弱符号

即使不手写 `weak`，C++ 里也有很多符号天然就是弱的，原因是它们可能在**多个翻译单元中重复定义**，而 ODR (One Definition Rule) 允许这样：

- `inline` 函数（包括类内定义的成员函数）；
- 函数模板 / 类模板成员的实例化；
- 类的虚函数表（vtable）、`typeinfo`；
- `inline` 变量（C++17）、类模板的静态成员。

{% include_code natural_weak_symbols.cpp lang:cpp tech/cpp/compile-link/weak-symbol-07.cpp %}

```bash
$ g++ -c natural_weak_symbols.cpp -o natural.o && nm -C natural.o
0000000000000000 W double add<double>(double, double)
0000000000000000 W int add<int>(int, int)
0000000000000000 T use()
0000000000000000 W twice(int)
```

`use()` 是普通函数，为强符号 `T`；`add<int>`、`add<double>`、`twice` 都是弱符号 `W`。这些符号还会被放进单独的 **COMDAT 段**（如 `.text._Z3addIiET_S0_S0_`），链接器据此去重，保证最终只保留一份。这正是多个 `.cpp` 都包含同一个头文件、里面有 `inline`/模板定义却不会报 `multiple definition` 的根本原因。

> 这类“天然弱符号”也是 ODR 违规难以被发现的原因：不同翻译单元里**定义不一致**的 inline 函数，链接器只会静默保留其中一份，不报错。


## 三、注意事项

1. **跨平台兼容性**：`__attribute__((weak))` 是 GCC / Clang 的扩展。MSVC 没有直接对应物，类似功能可用 `/alternatename` 链接选项、`#pragma comment(linker, "/include:...")` 或 DLL 导出 / 导入机制，语法不同。
2. **C++ 名称修饰（Name Mangling）**：C++ 支持函数重载，函数名会被编译器修饰。对普通 C++ 函数使用弱符号时，通常需要 `extern "C"` 避免修饰，保证链接器按同一个符号名匹配。
3. **使用弱引用前必须判空**：未定义的弱引用地址为 0，直接调用会导致段错误。
4. **弱引用不会触发静态库成员提取**：若可选符号的定义放在 `.a` 中，链接器不会为了解决弱未定义引用而拉入对应的 `.o`，需要直接链接 `.o`，或使用 `-Wl,--whole-archive`。

{% include_code lib_opt.cpp tech/cpp/compile-link/weak-symbol-08.cpp %}

{% include_code main_opt.cpp tech/cpp/compile-link/weak-symbol-09.cpp %}

```bash
$ g++ -c lib_opt.cpp -o lib_opt.o     # 里面有 optional_feature 的强定义
$ ar rcs libopt.a lib_opt.o

# 通过静态库链接：定义没有被拉入，弱引用保持为 0
$ g++ main_opt.cpp -L. -lopt -o ar1 && ./ar1
optional_feature not linked, skip

# 直接链接 .o：正常生效
$ g++ main_opt.cpp lib_opt.o -o ar2 && ./ar2
optional_feature from archive
```

所以如果希望“可选功能”放在静态库里并且被识别，需要：

- 直接链接 `.o`；
- 或使用 `-Wl,--whole-archive -lopt -Wl,--no-whole-archive`；
- 或在别处增加一个强引用（如 `-Wl,-u,optional_feature`）。


## 四、小结

- 弱符号是链接器层面的概念，通过 `__attribute__((weak))` 声明。
- 规则：强强冲突报错；强弱共存取强；仅有弱符号则用弱符号；未定义的弱引用地址为 0。
- 典型用途：库提供可被覆盖的默认实现（钩子、回调、插件机制）。
