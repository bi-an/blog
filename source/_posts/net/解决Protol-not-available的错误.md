---
title: 解决Protol not available的错误
date: 2025-08-15 10:45:33
tags: tcp/ip
---

## 问题描述

代码：

```cpp
getaddrinfo(localhost, "sunrpc", &hints, &result); // "sunrpc" 或 端口 111 都可以
for (const auto *rp = result; rp != nullptr; rp = rp->ai_next) {
    sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sock == -1) continue;
    if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1) break;  // Success
    perror("connect failed");
    close(sock);
    sock = -1;
}
```

这段代码，`connect()` 返回 `-1` ，报错为 `Protol not available` 。
- 但是我们明明是通过 `getaddrinfo()` 查询到的地址信息，为什么报错？
- 这个报错也很具有误导性，我们开启了 `IPv4` ，没有开启 `IPv6`，如果开启 `IPv6`，这个错误会消失，不支持 `IPv4` ？

## 排查过程

```bash
$ hostname
$ cat /etc/hosts
# Created by chnetinfo.sh.
# Do not remove the following line, or various programs
# that require network functionality will fail.
::1       localhost.localdomain   localhost
xx.xxx.xxx.xx   xiaoming-PC      xiaoming-PC.xiaoming-company.com
```

注：上面 `cat /etc/hosts` 的第二个 IP 进行了个人信息隐藏。`xx` 代表数字。

说明：

|    地址   | 协议版本 |              作用              |
|:---------:|:--------:|:------------------------------:|
| 127.0.0.1 | IPv4     | 本地回环地址，用于本机内部通信 |
| ::1       | IPv6     | 本地回环地址，用于本机内部通信 |

于是问题找到了：`/etc/hosts/` 只配置了 `::1` ，那么使用 IPv4 协议尝试 connect localhost 就会返回 -1 。



