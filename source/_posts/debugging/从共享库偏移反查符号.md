---
title: 从共享库偏移反查符号（Linux / ELF）
date: 2026-04-22 14:20:41
categories:
tags:
---

**变量约定：**

- `LIB`：要解析的 `*.so` 绝对或相对路径（请换成你的文件）。
- `OFF`：栈里 `+0x...` 的十六进制偏移；下文示例中 **`OFF = 0x8fa860`**（与 `addr2line` 所用一致）。
- `PC`：栈里 `[0x...]` 的**进程内**虚拟地址；示例 **`PC = 0x15182684b860`**（会随每次运行/ASLR 变化
  ，仅为与下组 `BASE` 配套的一例）。
- `BASE`：与上式满足 **`PC - BASE = OFF`** 的映射基址；示例 **`BASE = 0x151825f51000`**。具体取
  `maps` 中哪一段以实际进程为准。

---

## 场景

栈回溯里常见这种形式：

```text
/path/to/libapp.so(+0x8fa860)[0x15182684b860]
```

含义：

- **`libapp.so`**：出问题的共享库（例名，请替换为实际名）。
- **`+0x8fa860`**：相对该次映射**基址**的偏移（**示例中固定为上述 `OFF`**）。
- **`[0x15182684b860]`**：当时的绝对虚拟地址（**示例**；单独用时需配合 `maps` 中的 `BASE`）。

调试时通常先用 **`+0x...` 偏移** 对 **磁盘上的 `.so` 文件** 做解析。

---

## 首选：`addr2line`

在带有 **debug info**（`-g`，且未 strip 掉调试信息）的 `.so` 上：

```bash
addr2line -e /path/to/libapp.so -f -C 0x8fa860
```

**形态示例（你本机实跑时会是你的函数与路径）：**

```text
myapp::CommandHandler(int, char const**, long, char const*)
/path/to/src/cli/handler.cpp:42
```

`addr2line` 可能把路径记成**编译时目录**（如 `out/../handler.cpp`），以 DWARF 记录为准；若行号对不上
，多半是**二进制与当前源码**不是同一版构建。

| 选项      | 作用                           |
| --------- | ------------------------------ |
| `-e FILE` | 指定 ELF（`.so` 或可执行文件） |
| `-f`      | 同时打印函数名                 |
| `-C`      | Demangle C++ 符号              |

### 从「PC + 映射基址」反算并调用（注意 64 位运算）

`addr2line` 需要的是**相对**该 ELF **文件布局**的偏移，通常就是栈上 `+0x...`，此处即
**`0x8fa860`**。若你只有 `PC` 和 `BASE`，应得到 `OFF = PC - BASE`（在 64 位下计算）。

**Bash 的 `$(( ... ))` 对很大的十六进制字面量会溢出**；大地址用 Python 等做减法后再喂给
`addr2line`。

**脚本形态（数值与上节 `PC` / `OFF` / `BASE` 一致）：**

```bash
python3 <<'PY'
import subprocess
so = "/path/to/libapp.so"  # 换成你的 LIB
pc = 0x15182684b860
off = 0x8fa860             # 与栈中 +0x8fa860 相同
base = pc - off
print(f"BASE = 0x{pc:x} - 0x{off:x} = 0x{base:x}")
subprocess.run(["addr2line", "-e", so, "-f", "-C", hex(pc - base)], check=False)
PY
```

**形态示例（`BASE` 行与上式为算术一致的一例；函数名/路径为占位）：**

```text
BASE = 0x15182684b860 - 0x8fa860 = 0x151825f51000
myapp::CommandHandler(int, char const**, long, char const*)
/path/to/src/cli/handler.cpp:42
```

当 `pc - base` 与栈中 `+0x8fa860` 一致时，与直接 `addr2line -e ... -C 0x8fa860` 等价。

---

## 补充：`nm` / `objdump`（看落在哪个符号里）

无行号信息或想确认符号边界时，可按地址排序查看。全量往往很长，可只取前几行，或用 `grep` 搜**已知**子
串（来自 demangle 后的名字片段）。

```bash
nm -n --defined-only /path/to/libapp.so | head -5
```

**形态示例：**

```text
0000000000000000 n _GLOBAL_OFFSET_TABLE_
000000000000a000 t _ZN3App6WidgetC1Ev
000000000000a030 t _ZN3App6WidgetD1Ev
000000000000a060 t _ZN3App4InitEi
000000000000a090 t _ZN3App3RunEv
```

粗判：在排序列表里找 **地址不大于目标 `0x8fa860` 的最后一个**对应函数符号。精确行号仍以 `addr2line`
为准。

符号条数可能很大，例如某次对**同一类大 `.so`** 的统计（仅作数量级参考）：

```bash
nm -n --defined-only /path/to/libapp.so | wc -l
```

**形态示例：**

```text
86817
```

用 `objdump` 在符号表里按**短关键字**辅助定位；下面地址与**偏移 `0x8fa860`** 配套（符号起点略小于该
PC，落在函数体内）：

```bash
objdump -t /path/to/libapp.so | grep 'CommandHandler' | head -3
```

**形态示例：**

```text
00000000008fa71a l     F .text	0000000000000abc              _ZN5myapp15CommandHandlerEiPPKclS1_
```

行首 `0x8fa71a` 为该符号的**起点**，大小 `0xabc`；`0x8fa860` 落在区间 `[0x8fa71a, 0x8fa71a+0xabc)`
内。精确行号仍以 `addr2line` 为准。

### `readelf`：查看 LOAD 段

下面来自与上述 **`0x8fa860`** 同一 ELF 样例的 **`readelf -l | head -20`**（段尺寸因库而异，此组与示
例偏移**同时出现**时便于对照；若你换库则整段以本机为准）：

```bash
readelf -l /path/to/libapp.so | head -20
```

**配套示例输出：**

```text

Elf file type is DYN (Shared object file)
Entry point 0x0
There are 9 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  LOAD           0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x00000000007aa408 0x00000000007aa408  R      0x1000
  LOAD           0x00000000007ab000 0x00000000007ab000 0x00000000007ab000
                 0x00000000007bf251 0x00000000007bf251  R E    0x1000
  LOAD           0x0000000000f6b000 0x0000000000f6b000 0x0000000000f6b000
                 0x0000000000317c05 0x0000000000317c05  R      0x1000
  LOAD           0x0000000001282e20 0x0000000001283e20 0x0000000001283e20
                 0x00000000000c67c8 0x00000000000e2b68  RW     0x1000
  DYNAMIC        0x00000000012ec758 0x00000000012ed758 0x00000000012ed758
                 0x0000000000000530 0x0000000000000530  RW     0x8
  NOTE           0x0000000000000238 0x0000000000000238 0x0000000000000238
                 0x0000000000000024 0x0000000000000024  R      0x4
```

不同 `readelf` 版本与不同 ELF 细节可能略有差异，以**本机实跑**为准。

---

## C++ 符号 demangle

```bash
echo '_ZN5myapp15CommandHandlerEiPPKclS1_' | c++filt
```

**形态示例：**

```text
myapp::CommandHandler(int, char const**, long, char const*)
```

（上式 mangled 名须换成你从 `nm`/`objdump`/栈上看到的真实字符串。）

---

## 文件是符号链时

```bash
file /path/to/libapp.so
```

**形态示例：**

```text
/path/to/libapp.so: symbolic link to ../../build/obj/libapp.so
```

`addr2line` 应针对**实际打开的那份 ELF**（解析符号链后的目标或你确认参与调试的那份）；若安装目录下链
到构建树，以 inode 与更新时间为准，避免对旧副本解析。

---

## 常见问题

1. **`??` 或明显不对**

   - 二进制被 strip、或 debug 在单独文件且工具未找到 → 需带 debug 的构建物或 `DEBUGINFOD` 等。

2. **行号与当前仓库不一致**

   - 对应栈的二进制与当前源码**版本/配置**不同 → 以**构建该库**时的源码为准。

3. **大地址用 Bash 做 `PC - BASE`**
   - 易 32 位溢出；用 `python3` 或能表示 64 位无符号/有符号整数的工具计算后再调 `addr2line`。

---

## 最小复现（变量 + 一次 `addr2line`）

```bash
SO=/path/to/libapp.so
OFF=0x8fa860

addr2line -e "$SO" -f -C "$OFF"
```

**形态示例：**

```text
myapp::CommandHandler(int, char const**, long, char const*)
/path/to/src/cli/handler.cpp:42
```

将 `SO` 换成你的库路径；**偏移**若与栈上一致，可继续用 **`0x8fa860`** 作对照。函数名、源路径与行号以
你本机 `addr2line` 输出为准（受调试信息与构建一致性约束）。
