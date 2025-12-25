---
title: CLI11-命令行解析库的使用
date: 2025-12-25 15:05:48
categories:
tags: c/cpp
---

## CLI11 简介

CLI11 是一个用于处理命令行参数和选项的 C++ 库，旨在简化 C++ 应用程序的命令行界面开发。其主要特点包括：

1. **简单易用**：提供直观的 API，使开发者能够轻松定义和解析命令行选项
2. **现代 C++ 支持**：充分利用现代 C++ 特性，如类型推导和 lambda 表达式
3. **丰富的选项支持**：支持标志选项、位置参数、可选参数和必选参数等
4. **类型安全**：在解析和处理命令行参数时提供类型安全的机制
5. **灵活的错误处理**：提供多种错误处理方式，包括参数验证失败时的错误提示和帮助信息的自动生成
6. **跨平台支持**：可在主流操作系统上运行，包括 Windows、macOS 和各种 Linux 发行版

### 下载和安装

CLI11 是一个单头文件库，安装非常简单。有以下几种安装方式：

#### 方式一：单文件头文件（推荐）

1. 从 [CLI11 GitHub 仓库](https://github.com/CLIUtils/CLI11) 下载最新的 `CLI11.hpp` 文件
2. 将 `CLI11.hpp` 复制到您的项目包含目录中
3. 在代码中直接包含即可使用：
   ```cpp
   #include "CLI11.hpp"
   ```

#### 方式二：使用 CMake 集成

如果您的项目使用 CMake，可以通过以下方式集成：

1. **作为 Git 子模块**：
   ```bash
   git submodule add https://github.com/CLIUtils/CLI11.git
   ```
   在 `CMakeLists.txt` 中：
   ```cmake
   add_subdirectory(CLI11)
   target_link_libraries(your_target CLI11::CLI11)
   ```

2. **使用 FetchContent**（CMake 3.11+）：
   ```cmake
   include(FetchContent)
   FetchContent_Declare(
     CLI11
     GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
     GIT_TAG        v2.4.1  # 使用最新版本标签
   )
   FetchContent_MakeAvailable(CLI11)
   target_link_libraries(your_target CLI11::CLI11)
   ```

#### 方式三：包管理器安装

- **vcpkg**：`vcpkg install cli11`
- **Conan**：`conan install CLI11/2.4.1@cliutils/stable`
- **Homebrew**（macOS）：`brew install cli11`

#### 方式四：全局安装

将 `CLI11.hpp` 复制到系统共享文件夹位置（如 `/opt/CLI11` 或 `/usr/local/include`），然后在 CMake 中：
```cmake
include_directories(/opt/CLI11)
```

**注意**：`CLI11.hpp` 包含整个命令行解析库的核心功能。如果需要使用单独的实用工具（如 Timer、AutoTimer），需要单独复制相应的头文件。

## CLI11 API 层级关系

### CLI11 的核心数据结构

CLI11 有三个核心数据结构：

1. **`App`** - 应用根对象
   - 所有选项和子命令的容器
   - 代表整个命令行应用程序
   - **Subcommand** 实际上就是 `App` 对象，不是独立的数据结构。
     - `add_subcommand()` 返回 `App*`，所以 Subcommand 可以无限嵌套。

2. **`Option`** - 选项对象
   - 通过 `add_flag()` 或 `add_option()` 创建
   - **`add_flag()`** → 返回 `Option*`
     - 布尔标志，不需要值
     - 示例: `app.add_flag("-v", verbose)`
   - **`add_option()`** → 返回 `Option*`
     - 需要值的选项
     - 示例: `app.add_option("-f", file, "File path")`

3. **`Option_group`** - 选项组（继承自 `App`）
   - **继承关系**: `class Option_group : public App`
   - 本质上是 `App` 对象，可以使用 `App` 的所有方法
   - 用于组织相关选项
   - 通过 `add_option_group()` 创建，返回 `Option_group*`（可转换为 `App*`）
   - 用于实现 Suboption 功能
   - **嵌套支持**: 因为继承自 `App`，可以在 `Option_group` 中再创建 `Option_group` 实现多层嵌套

### 层级结构

#### 层级 0: App（应用根对象）
```cpp
CLI::App app("description");
```
- 所有选项和子命令的顶层容器

#### 层级 1: App 的直接子级（三种平级对象）

**Option（选项）** - 通过 `add_flag()` 或 `add_option()` 添加
- `add_flag()`: 布尔标志，不需要值
- `add_option()`: 需要值的选项
- 两者都返回 `Option*`，独立存在

**Subcommand（子命令）** - 通过 `add_subcommand()` 添加
- 返回 `App*` 对象，可以继续调用 `add_subcommand()` 实现无限嵌套
- 独立存在，本身也是 App 类型

**Option_group（选项组）** - 通过 `add_option_group()` 添加
- 返回 `Option_group*`（继承自 App）
- 用于组织选项，实现 Suboption 功能

#### 层级 2: 依赖关系

**Suboption（子选项）** - 通过 `Option_group + needs()` 实现
```cpp
app.add_flag("-add", add_flag);  // 父选项
CLI::Option_group *add_group = app.add_option_group("add_suboptions", "Sub-options for -add");
add_group->add_option("-file", file_path);  // 子选项
add_group->needs(app.get_option("-add"));  // 建立依赖
```
- 使用: `myprog -add -file path.txt`（必须先有 `-add`）
- 多层嵌套: 在 `Option_group` 中再创建 `Option_group`

**Subcommand 的 Option** - 属于 Subcommand
```cpp
CLI::App *start = app.add_subcommand("start");
start->add_option("-f", file_path);  // 子命令的选项
```
- 使用: `myprog start -f file.txt`

### 关键区别

| 类型 | API 函数 | 返回类型 | 层级 | 独立性 | 示例 |
|------|---------|---------|------|--------|------|
| **App** | `CLI::App app("desc")` | `App` | 0 | 根对象 | 应用根对象 |
| **Option (flag)** | `app.add_flag()` | `Option*` | 1 | 独立 | `myprog -v` |
| **Option (option)** | `app.add_option()` | `Option*` | 1 | 独立 | `myprog -f file.txt` |
| **Subcommand** | `app.add_subcommand()` | `App*` | 1 | 独立，可嵌套 | `myprog start` |
| **Suboption** | `Option_group + needs()` | - | 2 | 依赖父 Option | `myprog -add -file path.txt` |
| **Subcommand 的 Option** | `subcmd->add_option()` | `Option*` | 2 | 属于 Subcommand | `myprog start -f file.txt` |

## 一个 Suboption 的示例程序

以下是一个 Suboption 的示例程序，**不支持子命令**，**使用单横线作为长选项**，**支持子选项**。
我们选择这个示例是因为 Subcommand 的实现比较简单（就是 App 的无限嵌套），所以我们写了一个支持子选项的示例程序来演示更复杂的用法。

### 示例程序的层级结构

示例程序演示了**三级层级**结构：

```
App (myprog)
├── Option (-add)          ← Level 1: 顶级选项 (flag)
├── Option (-del)          ← Level 1: 顶级选项 (flag, 与 -add 平级)
├── Option (-force)        ← Level 1: 顶级选项 (flag, 与 -add 平级)
└── Option_group (add_suboptions)  ← Level 2: 子选项组
    ├── Option (-file)     ← Level 2: -add 的子选项 (需要值)
    ├── Option (-recursive)← Level 2: -add 的子选项 (flag, 不需要值)
    └── Option_group (file_suboptions)  ← Level 3: -file 的子选项组
        ├── Option (-encoding)   ← Level 3: -file 的子选项 (需要值)
        └── Option (-overwrite)  ← Level 3: -file 的子选项 (flag, 不需要值)
```

依赖关系通过以下方式建立：
- `add_group->needs(add_option)` - 使选项组要求 `-add` 选项必须存在
- `file_group->needs(file_option)` - 使文件子选项组要求 `-file` 选项必须存在

**示例命令行：**
- 仅 Level 1: `myprog -add -del -force`
- Level 1 + 2: `myprog -add -file path/to/file.txt -recursive -del -force`
- Level 1 + 2 + 3: `myprog -add -file path/to/file.txt -encoding utf8 -overwrite -del -force`

### 文件说明

#### suboption_example.cpp - 示例程序

`suboption_example.cpp` 是一个可执行的示例程序，用于演示子选项功能。可以直接运行并测试功能。

 {% include_code lang:cpp CLI11/suboption_example.cpp %}

##### 构建和运行示例程序

```bash
cd /home/zhigaoz/code/CLI11/build
cmake --build . --target suboption_example
# Run examples
./tests/suboption_test/suboption_example --help
./tests/suboption_test/suboption_example -add
./tests/suboption_test/suboption_example -add -file path/to/file.txt -encoding utf8 -overwrite -del -force
```

### 实现细节

测试使用 CLI11 的 `Option_group` 功能和 `needs()` 方法，确保子选项只有在父选项存在时才有效。

#### 关键实现细节

1. **非标准选项名**: 
   - 使用 `app.allow_non_standard_option_names()` 允许单破折号后面跟多个字符的选项（例如 `-add` 而不是 `--add`）

2. **选项组创建**:
   ```cpp
   CLI::Option_group *file_group = add_group->add_option_group("file_suboptions", "Sub-options for -file");
   file_group->allow_non_standard_option_names();
   ```

3. **依赖关系**:
   ```cpp
   CLI::Option *add_option = app.get_option("-add");
   add_group->needs(add_option);  // 子选项需要 -add 选项存在
   ```

4. **添加子选项**:
   ```cpp
   // 注意：CLI11 没有 add_suboption() 函数
   // 子选项通过在 Option_group 中使用 add_option() 或 add_flag() 实现
   add_group->add_option("-file", file_path, "File path (requires a value)");
   add_group->add_flag("-recursive", recursive_flag, "Process recursively (flag, no value needed)");
   ```
