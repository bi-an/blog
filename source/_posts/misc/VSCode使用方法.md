---
title: VSCode 使用方法
date: 2024-01-24 17:26:44
categories: Misc
tags: vscode
---

## Reduce the disk usage of VSCode

以下文件可以删除：

- .vscode-server/data/User/workspaceStorage/\*：几十 GB.
- .vscode-server：[参考](https://stackoverflow.com/questions/58453967/vscode-remote-ssh-vscode-server-taking-up-a-lot-of-space)
- .config: 可以针对每个目录找到相应的程序，比如 Chrome，然后有针对性地删除一些文件。参考
  [这里](https://stackoverflow.com/questions/58453967/vscode-remote-ssh-vscode-server-taking-up-a-lot-of-space)。
- .cache: 和 .config 类似处理，参考
  [这里](https://superuser.com/questions/366771/what-does-cache-do-it-consumes-a-huge-disk-space)。

  .cache/vscode-cpptools: This cache folder stores cached precompiled headers or "ipch" files that
  the IntelliSense engine uses to improve performance. You can limit the size of the cache by
  changing the setting value of C_Cpp.intelliSenseCacheSize. 参考
  [这里](https://github.com/microsoft/vscode-cpptools/issues/6594)：
  ![C_Cpp.intelliSenseCacheSize](image.png)

## Hot keys

1. 跳转到对应大括号：`Ctrl Shift \`

## Debug code in VSCode

- [Debug multiple processes in VSCode](https://code.visualstudio.com/Docs/editor/debugging#_multitarget-debugging)
- [Debug Python Subprocess](https://stackoverflow.com/questions/60515935/visual-studio-code-does-not-attach-debugger-to-multi-processes-in-python-using-p)

## Useful plugins

- `Remote - SSH`: Windows 连接 Linux。前置条件：Windows 本身需要安装 ssh 命令行工具。
- `Perforce for VS Code`: perforce 工具。
- `intent-rainbow`: 用彩色显示缩进。
- `C/C++`
- `Makefile Tools`
- `Verilog Format`
- `All in one`
- `c/c++ definition generator`
- `KoroFileHeader`: 参考 [这里](https://zhuanlan.zhihu.com/p/610490070)。

## Useful configurations

[自动填充头文件，比如 ifndef 宏](https://www.cxyzjd.com/article/weixin_45461426/105936955)

配置文件：

- launch.json
- tasks.json:
  [ref](https://stackoverflow.com/questions/48273346/vscode-command-for-user-input-in-debug-launch-config),
  [ref](https://code.visualstudio.com/updates/v1_30#_improved-user-input-variables-for-task-and-debug-configurations)

## 使用 VSCode 合并 Perforce 代码

在 `~.cshrc` 脚本中添加：

```bash
## Use VSCode to diff/merge the Perforce code
#
# Refer to:
#  https://github.com/mjcrouch/vscode-perforce/issues/259
#  https://www.perforce.com/manuals/cmdref/Content/CmdRef/p4_resolve.html
#
# Map p4 resolve arguments to vscode's merge editor arguments
#
# 'code' takes: <path1> <path2> <base> <result>
#
# perforce gives:
#   1 the base file
#   2 the source file (also known as "theirs")
#   3 the target file (also known as "yours")
#   4 the merge file.
setenv P4MERGE 'code --wait --merge $2 $3 $1 $4'
setenv P4DIFF 'code --wait --diff'
setenv P4EDITOR 'code --wait'
```

## 跳转到定义如何加速

🚀 解决方案一：切换 IntelliSense 引擎 VSCode 默认使用的是 “Default” 引擎，它功能强但对大型项目解析慢
。你可以改成更轻量的 Tag Parser：

操作步骤：打开 VSCode 设置（Ctrl + ,）

搜索 C_Cpp.intelliSenseEngine

将值从 "Default" 改为 "Tag Parser"

这样跳转速度会明显提升，尤其在大型项目中。

⚙️ 解决方案二：优化智能提示延迟在设置中调整提示延迟也能改善体验：

搜索 Editor: Quick Suggestions Delay，设为 10ms 以下

搜索 Snippets Prevent Quick Suggestions，设为 false

这些设置可以让提示和跳转更流畅。

🧠 额外建议关闭未使用的扩展：某些扩展会拖慢性能

使用 compile_commands.json：如果你用 CMake，可以生成这个文件，让 VSCode 更准确地解析项目结构

避免打开整个大型项目目录：只打开你正在编辑的子目录，有助于减少分析负担

### compile_commands.json

🔧 Clang 生态工具（最主要用户）

这些工具依赖 compile_commands.json 来正确解析代码：

- Clangd：语言服务器，提供 VSCode、Vim、Emacs 等编辑器的智能补全、跳转、诊断等功能。
  - clangd 比 Microsoft 的 C/C++ 插件跳转更快，它可以手动安装到系统，也可以直接用 VSCode 的
    Extensions 市场安装（Remote SSH 也支持）。
  - compile_commands.json 是 clangd 的眼睛，把这个文件放在项目根目录，可以显著提高跳转速度。
- Clang-Tidy：静态分析工具，用于检查代码质量、风格、潜在错误
- Include-What-You-Use (IWYU)：分析并清理多余的 #include 指令
- Clang-Format：在复杂项目中更好地格式化代码

🖥️ 智能 IDE 和编辑器 VSCode：

通过 C/C++ 插件读取 compile_commands.json 来加速跳转和补全

- CLion、QtCreator：这些 IDE 会自动识别并使用该文件来构建项目模型

🛠️ 构建工具和生成方式 CMake：

最常见的生成方式，只需添加参数：

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

就会在构建目录中生成该文件

Bear 工具：适用于非 CMake 项目（如 Makefile），通过拦截编译命令生成该文件
