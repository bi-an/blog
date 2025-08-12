---
title: Bazel入门
date: 2025-08-12 20:21:42
tags: c/cpp
---

## Bazel简介

Bazel 是 Google 开发并开源的一款构建工具（build system），主要用来自动化编译、测试和打包大型软件项目。

- 高速构建：相当于智能化的Make
- 可复现构建：通过严格的声明式 BUILD 文件，确保每次构建结果一致。
- 跨平台：支持 Linux、macOS、Windows 等多个操作系统。
- 支持多语言：不只支持 C++，还支持 Java、Python、Go、Shell 等多种语言的项目构建。


## 安装Bazel

### 方法1：apt 镜像源

清华 apt 镜像源：

```bash
sudo tee /etc/apt/sources.list.d/bazel.list <<EOF
deb [arch=amd64] https://mirrors.tuna.tsinghua.edu.cn/bazel-apt stable jdk1.8
EOF
```
注：`jdk1.8`是`https://mirrors.tuna.tsinghua.edu.cn/bazel-apt/dists/stable/`下的一个目录。
如果提示找不到对应的路径，就去查看该目录是否存在。

导入清华镜像的 GPG key：

```bash
curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/bazel.gpg > /dev/null
```

更新并安装：

```bash
sudo apt update
sudo apt install bazel
bazel --version
```

### 方法 2：直接下载二进制（绕过 apt）

从 GitHub Release 下载：

```bash
wget https://github.com/bazelbuild/bazel/releases/download/6.5.0/bazel-6.5.0-linux-x86_64
chmod +x bazel-6.5.0-linux-x86_64
sudo mv bazel-6.5.0-linux-x86_64 /usr/local/bin/bazel
bazel --version
```

### 方法 3：用 Bazelisk 自动下载版本

```bash
wget https://github.com/bazelbuild/bazelisk/releases/download/v1.21.0/bazelisk-linux-amd64
chmod +x bazelisk-linux-amd64
sudo mv bazelisk-linux-amd64 /usr/local/bin/bazel
bazel --version
```

第一次运行时会自动拉取所需版本。

## 使用Bazel

| 命令                     | 作用         |
| ---------------------- | ---------- |
| `bazel build //target` | 构建指定目标     |
| `bazel test //target`  | 运行指定测试     |
| `bazel run //target`   | 构建并运行可执行文件 |
| `bazel clean`          | 清理构建缓存     |

### 项目结构示例

假设你有一个 C++ 项目，目录结构：

```css
WORKSPACE
BUILD
main.cpp
hello.h
hello.cpp
hello_test.cpp
```

- WORKSPACE 文件：标记项目根目录，Bazel 必需。内容可以为空。
- BUILD 文件：描述如何构建项目。

### 写一个简单 BUILD 文件

{% include_code lang:bazel bazel_hello/BUILD %}

### 简单源码示例

{% include_code lang:cpp bazel_hello/hello.h %}

{% include_code lang:cpp bazel_hello/hello.cpp %}

{% include_code lang:cpp bazel_hello/hello_test.cpp %}

{% include_code lang:cpp bazel_hello/main.cpp %}

### 构建和测试

- 编译可执行文件
```bash
bazel build //:hello_bin
```
其中，冒号用于分隔路径和目标。

- 运行可执行文件
```bash
bazel run //:hello_bin
```

- 运行测试
```bash
bazel test //:hello_test
```
