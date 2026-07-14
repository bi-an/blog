---
title: CUDA矩阵乘法
date: 2025-09-03 09:23:17
categories: cuda
tags:
 - cuda
 - gpu
---


## 使用共享内存优化

<a href='https://postimg.cc/BjfXcPQ9' target='_blank'><img src='https://i.postimg.cc/dVLrFGb1/matrix-Mul.png' border='0' alt='matrix-Mul'/></a>

{% include_code cuda-matrix-multiplication-01.cpp:54-126 lang:cpp from:54 to:126 tech/cuda/cuda-matrix-multiplication-01.cpp %}
