---
title: Introduction to memory
date: 2024-01-29 14:25:24
categories: c/cpp
tags: memory
---

## malloc/free

See [this example](https://www.gnu.org/software/libc/manual/html_node/Backtraces.html)

```cpp
char ** backtrace_symbols (void *const *buffer, int size) 
```

> The return value of backtrace_symbols is a pointer obtained via the malloc function, and it is the responsibility of the caller to free that pointer. Note that only the return value need be freed, not the individual strings.

Question: Why does it say "only the return value need be freed, not the individual strings"?

Let us observe the defintion of the `malloc`/`free` functions first:

```cpp
void *malloc( size_t size );
void free( void *ptr );
```

`free` takes a `void*` pointer to deallocate the memory, it doesn't care what type it is, even if it is a multi-level pointer. It means that `malloc` has stored the memory size in some place and `free` will find it beforing deallocate the memory.

Let us return the question. The memory pointer returned by `backtrace_symbols` is the `char**` type, it must be a whole block contigunous memory using `malloc` and might be enforced to be transformed as `char**` pointer when returing. So when we `free` the memory block, the Linux kernel find its actual memory size and deallocate it.

Example:

```cpp
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    char** strings = (char**)malloc(3 * sizeof(char*) + 3 * 50); // assuming a maximum 50 characters per sentence
    char* block = (char*)(strings + 3);
    char* s1 = strcpy(block, "The first sentence"); block += strlen(s1) + 1;
    char* s2 = strcpy(block, "The second sentence"); block += strlen(s2) + 1;
    char* s3 = strcpy(block, "The third sentence");
    strings[0] = s1;
    strings[1] = s2;
    strings[2] = s3;
    for(int i = 0; i < 3; ++i) {
        printf("%s\n", strings[i]);
    }
    free(strings); // deallocate all memory at once

    return 0;
}
```

More elegant but less economical code:

```cpp
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    char** strings = (char**)malloc(3 * sizeof(char*) + 3 * 50);
    char* block = (char*)(strings + 3);
    for(int i = 0; i < 3; ++i) {
        strings[i] = block + i * 50; // Assuming a maximum of 50 characters per sentence
    }
    strcpy(strings[0], "The first sentence");
    strcpy(strings[1], "The second sentence");
    strcpy(strings[2], "The third sentence");

    for(int i = 0; i < 3; ++i) {
        printf("%s\n", strings[i]);
    }

    free(strings); // deallocate all memory at once

    return 0;
}
```

## Reference

* [The GNU C Library (glibc) manual: 13.8 Memory-mapped I/O](https://www.gnu.org/software/libc/manual/html_node/Memory_002dmapped-I_002fO.html)
* [The GNU C Library (glibc) manual: 3.4 Memory Protection](https://www.gnu.org/software/libc/manual/html_node/Memory-Protection.html)
