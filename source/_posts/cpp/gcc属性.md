---
title: gcc属性
date: 2025-07-25 11:53:54
tags: c/cpp
---


## `__attribute__((__visibility__("default")))`

`__attribute__((__visibility__("default")))` 是 GCC 编译器的一个属性，用于控制符号的可见性。它指定了一个符号（如函数或变量）在共享库中的可见性。

### 含义：
- **`default` 可见性**：
  - 符号可以被其他共享库或可执行文件访问。
  - 符号会被导出到共享库的符号表中。

### 用途：
- 用于显式导出符号，使其在动态链接时可被其他模块使用。

- 在共享库中，默认情况下符号是 `default` 可见的，但使用 `-fvisibility=hidden` 编译选项时，所有符号会被隐藏，只有显式标记为 `default` 的符号才会导出。

- 编译器优化的影响：即使没有 `-fvisibility=hidden`，某些编译器优化（如 `-flto`）可能会影响符号的导出。
显式使用 `__attribute__((__visibility__("default")))` 可以避免这些问题。

### 示例：
```cpp
__attribute__((__visibility__("default")))
void myFunc() {
    // Function implementation
}
```


---


可以通过以下步骤确认 `myFunc` 是否在被链接时可见：

### 方法 1: 使用 `nm` 工具检查符号
`nm` 是一个工具，可以列出目标文件或共享库中的符号表。

1. 检查 `libchip.so` 中是否导出了 `myFunc`：
   ```bash
   nm -D libchip.so | grep myFunc
   ```
   - 如果 `myFunc` 出现在输出中，说明它被导出并且可见。
   - 如果没有出现，可能是符号被隐藏（例如使用了 `-fvisibility=hidden`）。

2. 如果 `myFunc` 没有导出：
   - 确认是否在代码中使用了 `__attribute__((__visibility__("default")))`。
   - 确认编译时是否启用了 `-fvisibility=hidden`。

---

### 方法 2: 使用 `readelf` 检查动态符号
`readelf` 可以显示共享库的动态符号表。

1. 检查 `libchip.so` 的动态符号表：
   ```bash
   readelf -Ws libchip.so | grep myFunc
   ```
   - 如果 `myFunc` 出现在输出中，说明它是动态可见的。
   - 如果没有出现，说明符号被隐藏。

---

### 方法 3: 检查链接依赖关系
1. 检查 `libdbg.so` 是否声明了对 `libchip.so` 的依赖：
   ```bash
   readelf -d libdbg.so | grep NEEDED
   ```
   - 如果 `libchip.so` 出现在 `NEEDED` 列表中，说明 `libdbg.so`依赖 `libchip.so`。

2. 如果 `libchip.so` 在依赖列表中，但链接时仍然报错：
   - 确认 `myFunc` 是否在 `libchip.so` 中导出（使用 `nm` 或 `readelf` 检查）。

---

### 方法 4: 检查链接器错误
如果链接器报错 `undefined reference to myFunc`：
- 确认链接命令是否显式包含 `libchip.so`：
  ```bash
  g++ -o executable main.o -L/path/to/libs -ldbg -lchip
  ```
- 如果没有显式链接 `libchip.so`，动态链接器可能无法解析 `myFunc`。

---

### 总结：
- 使用 `nm` 或 `readelf` 检查 `libchip.so` 是否导出了 `myFunc`。
- 如果符号未导出，确保代码中使用了 `__attribute__((__visibility__("default")))` 并检查编译选项。