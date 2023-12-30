---
title: cmake usage
date: 2023-12-07 10:58:25
categories: Tools
tags: cmake
---

Specify the search path for `pkg_check_modules`: [Solution](https://stackoverflow.com/questions/44487053/set-pkg-config-path-in-cmake)

```cmake
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SOURCE_DIR}/libs/opencv-install/lib/pkgconfig")
```

