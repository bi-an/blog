---
title: VSCode Usage
date: 2024-01-24 17:26:44
categories: Misc
tags: vscode
---

## Reduce the disk usage of VSCode

以下文件可以删除：

- .vscode-server/data/User/workspaceStorage/*：几十GB.
- .vscode-server：[参考](https://stackoverflow.com/questions/58453967/vscode-remote-ssh-vscode-server-taking-up-a-lot-of-space)
- .config: 可以针对每个目录找到相应的程序，比如 Chrome，然后有针对性地删除一些文件。参考[这里](https://stackoverflow.com/questions/58453967/vscode-remote-ssh-vscode-server-taking-up-a-lot-of-space)。
- .cache: 和 .config 类似处理，参考[这里](https://superuser.com/questions/366771/what-does-cache-do-it-consumes-a-huge-disk-space)。

    .cache/vscode-cpptools: This cache folder stores cached precompiled headers or "ipch" files that the IntelliSense engine uses to improve performance. You can limit the size of the cache by changing the setting value of C_Cpp.intelliSenseCacheSize. 参考[这里](https://github.com/microsoft/vscode-cpptools/issues/6594)：
    ![C_Cpp.intelliSenseCacheSize](image.png)

## Hot keys

1. 跳转到对应大括号：`Ctrl Shift \`

## Debug code in VSCode

* [Debug multiple processes in VSCode](https://code.visualstudio.com/Docs/editor/debugging#_multitarget-debugging)
* [Debug Python Subprocess](https://stackoverflow.com/questions/60515935/visual-studio-code-does-not-attach-debugger-to-multi-processes-in-python-using-p)



## Useful plugins

- `Remote - SSH`: Windows 连接 Linux。前置条件：Windows 本身需要安装 ssh 命令行工具。
- `Perforce for VS Code`: perforce 工具。
- `intent-rainbow`: 用彩色显示缩进。
- `C/C++`
- `Makefile Tools`
- `Verilog Format`
- `All in one`
- `c/c++ definition generator`
- `KoroFileHeader`: 参考[这里](https://zhuanlan.zhihu.com/p/610490070)。

## Useful configurations

[自动填充头文件，比如ifndef宏](https://www.cxyzjd.com/article/weixin_45461426/105936955)

配置文件：
- launch.json
- tasks.json: [ref](https://stackoverflow.com/questions/48273346/vscode-command-for-user-input-in-debug-launch-config), [ref](https://code.visualstudio.com/updates/v1_30#_improved-user-input-variables-for-task-and-debug-configurations)
