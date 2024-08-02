---
title: VSCode Configuration
date: 2024-08-02 19:13:09
categories: script
tags: VSCode
---

## debug

在当前代码根目录下创建.vscode/launch.json文件，写入如下内容：

```json
{
    "configurations": [
        {
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/a.out",
            "args": [],
            "stopAtEntry": true,
            "cwd": "${workspaceFolder}",
            "miDebuggerServerAddress": "<remote host>:<remote port>",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
        },
        {
            "name": "(gdb) Attach",
            "type": "cppdbg",
            "request": "attach",
            # "program": "/path/to/executable",
            "program": "/proc/self/exe",
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ]
        }
    ],
    "version": "2.0.0"
}
```
