#include <stdio.h>

#include "fake_dlfcn.h"
#include "mylib.h"

int main() {
    // 使用 fake_dlopen 加载动态库
    void *handle = fake_dlopen("/home/bi-an/code/blog/source/downloads/code/compile/build/libmylib.so", 0);
    if (!handle) {
        fprintf(stderr, "Failed to load library\n");
        return 1;
    }

    // 使用 fake_dlsym 查找符号
    void (*hello)() = (void (*)())fake_dlsym(handle, "hello");
    if (!hello) {
        fprintf(stderr, "Failed to find symbol 'hello'\n");
        fake_dlclose(handle);
        return 1;
    }

    // 调用动态库中的函数
    hello();

    // 关闭动态库
    fake_dlclose(handle);
    return 0;
}