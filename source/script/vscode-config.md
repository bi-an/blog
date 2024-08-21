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
            // "program": "/path/to/executable",
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

其中，"(gdb) Launch"可以用来连接gdbserver。用gdbserver启动executable：

```bash
gdbserver peer_host:local_port prog [args]
```

如果在点击"Run"按钮之前，在VSCode的Debugger Terminal写上命令（可以不按Enter），那么在程序启动时，该命令会被先执行。


## Q & A

	1. Q: Configured debug type 'cppdbg' is not supported.
	A: Please check if the "c/c++" extension is installed. (ref)
	2. Q: VS Code Remote-SSH: The vscode server failed to start SSH
	A: https://stackoverflow.com/questions/67976875/vs-code-remote-ssh-the-vscode-server-failed-to-start-ssh
	3. Q: Where does VS Code store the data for its Local History feature?
	A: https://stackoverflow.com/questions/72610147/where-does-vs-code-store-the-data-for-its-local-history-feature
	4. vscode无法跳转到定义
	A: when "C_Cpp.intelliSenseEngine" is set to "Tag Parser", Goto definition is working fine.
https://github.com/microsoft/vscode-cpptools/issues/273