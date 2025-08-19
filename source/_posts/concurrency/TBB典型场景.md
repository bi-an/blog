---
title: TBB典型场景
date: 2025-08-19 22:48:38
categories:
tags: 线程
---

## IO + CPU 密集 + IO


{% include_code lang:cpp tbb/pipeline/1_message_queue.cpp %}

{% include_code lang:cpp tbb/pipeline/2_flow_graph.cpp %}

{% include_code lang:cpp tbb/pipeline/3_pipeline.cpp %}
