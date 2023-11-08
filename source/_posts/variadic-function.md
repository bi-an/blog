---
title: variadic function
tags:
  - cpp
  - c
date: 2023-11-03 18:52:32
---


## Variadic function in C

See more: [Variadic functions in C](https://www.geeksforgeeks.org/variadic-functions-in-c/)

示例1：`ap_list` + `vsnprintf`

```cpp
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h> // gettimeofday
#include <time.h>
#include <sys/socket.h>

using namespace std;

void foo(int id, const char *fmt, ...)
{
    constexpr int MAXLEN = 1024;
    char buf[MAXLEN];
    int n;

    n = snprintf(buf, MAXLEN, "INFO(%d): ", id);

    va_list ap;
    // ap will be the pointer to the last fixed argument of the variadic function.
    va_start(ap, fmt);
    n += vsnprintf(buf + n, MAXLEN - n, fmt, ap);
    // This ends the traversal of the variadic function arguments.
    va_end(ap);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    char timebuf[64];
    // size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
    snprintf(timebuf, MAXLEN - n, "%d-%02d-%02d %d:%d:%d.%ld %s",
             tm.tm_year + 1900, tm.tm_mon, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, tm.tm_zone);

    printf(buf, timebuf);
}

int main()
{
    const char* name = "Tony";
    // "%%s" is a placeholder of the timestamp for the vsnprintf function.
    foo(123, "Hello %s at %%s\n", name);
    // Will print:
    // INFO(123): Hello Tony at 2023-10-04 21:25:23.682853 CST

    return 0;
}
```

示例2：`va_list` + `va_arg`

```cpp
#include <stdarg.h>
#include <stdio.h>

constexpr bool debug = true;

int printDebugLog(const char* fmt, ...) {
    if (debug) {
        // The first argument doesn't need to traverse via va_list.
        printf("fmt=%s\n", fmt);
        va_list ap;
        va_start(ap, fmt);
        long int t = va_arg(ap, long int);
        va_end(ap);
        printf(fmt, t);
    }
    return 0;
}

int main() {
  long int time = 100;
  printDebugLog("Elapsed time is: %ld seconds.\n", time);

  return 0;
}
```

