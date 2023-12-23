---
title: Debugging-Tips
date: 2023-12-21 15:13:46
tags:
---

## gdb

```gdb
(gdb) breakpoint exit
(gdb) breakpoint _exit
(gdb) breakpoint atexit
(gdb) breakpoint abort
```


Enable coredump: [how to do](https://medium.com/@sourabhedake/core-dumps-how-to-enable-them-73856a437711)

```bash
ulimit -c unlimited
```

Where is the core dumped file:

```bash
grep 'kernel.core_pattern' /etc/sysctl.conf
```

## strace

Example:

```bash
strace -f -o strace.log -tt -y -yy -e trace=desc,process,network
```

Refer to [here](https://gist.github.com/graste/929bb122c353bdd90c20)

> -e trace=ipc – communication between processes (IPC)
> -e trace=memory – memory syscalls
> -e trace=network – network syscalls
> -e trace=process – process calls (like fork, exec)
> -e trace=signal – process signal handling (like HUP, exit)
> -e trace=file – file related syscalls
> -e trace=desc – all file descriptor related system calls

