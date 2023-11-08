---
title: 加密算法及例子
tags:
---

## github personal access token

github取消了https的用户名+密码验证方式，改用personal access token验证。请参考[官方文档](https://docs.github.com/zh/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens)。

命令行

    gh help auth
    gh auth status
    gh auth login -h github.com

`gh`生成的配置在`~/.config/gh/hosts.yml`文件中。



