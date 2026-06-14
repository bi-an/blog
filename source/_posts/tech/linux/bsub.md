---
categories: linux
date: 2023-12-06 09:41:56
tags:
- linux
- system
title: bsub用法
---

## find_host_queue.sh

寻找 hostname 所在的 queue ：

{% include_code tech/linux/bsub-01.sh %}

## auto_submit_to_host_queue.sh

根据提供的 hostname ，自动寻找最空闲的队列提交 bsub 作业：

{% include_code tech/linux/bsub-02.sh %}
