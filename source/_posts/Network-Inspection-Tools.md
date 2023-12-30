---
title: Network Inspection Tools
date: 2023-12-29 09:55:00
categories: Programming
tags: Tools
---

## Installation

    sudo apt install net-tools

## Manual

    man 7 socket - socket options
    man 7 tcp - tcp options

## Commands

### ip

NAME

    ip - show / manipulate routing, devices, policy routing and tunnels.

### netstat / ss

NAME

    ss - athoher to investigate sockets.

References

[查看进程占用的端口](https://zhuanlan.zhihu.com/p/45920111)

COMMANDS

    ss -lnpt

OPTIONS

    -l, --listening

    -n, --numberic

    -t, --tcp

    -u, --udp

    -p, --process

### lsof


