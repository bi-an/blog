---
title: bsub用法
categories: Network
tags: bsub
date: 2023-12-06 09:41:56
---

## find_host_queue.sh

寻找 hostname 所在的 queue ：

{% include_code lang:bash bsub/find_host_queue.sh %}

## auto_submit_to_host_queue.sh

根据提供的 hostname ，自动寻找最空闲的队列提交 bsub 作业：

{% include_code lang:bash bsub/auto_submit_to_host_queue.sh %}
