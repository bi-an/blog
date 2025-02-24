---
title: TCL入门
date: 2025-02-24 16:03:38
tags:
---

`Tcl_Main`的简化流程：

```cpp
void Tcl_Main(int argc, char *argv[], Tcl_AppInitProc *appInitProc) {
    Tcl_Interp *interp;

    // 创建一个新的 Tcl 解释器
    interp = Tcl_CreateInterp();

    // 调用应用程序初始化函数
    if ((*appInitProc)(interp) != TCL_OK) {
        fprintf(stderr, "Application initialization failed: %s\n", Tcl_GetStringResult(interp));
        exit(1);
    }

    // 进入 Tcl 事件循环
    Tcl_MainLoop();
}
```
