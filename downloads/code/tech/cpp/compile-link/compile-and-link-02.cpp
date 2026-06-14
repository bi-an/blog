#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*func_t)();  // 定义函数指针类型

int main() {
    // 打开 .so 文件
    void *handle = dlopen("your_library.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // 清除任何现有的错误
    dlerror();

    // 读取符号
    // void (*func)(); // 方法一：声明并定义函数指针
    func_t func;  // 方法二：定义函数指针
    // 以下四种方法/写法都可以用于将 dlsym 返回的 void* 赋值给函数指针
    // (1) *(void **) (&func) = dlsym(handle, "your_function");
    //    解释：取函数指针 func 的地址，将其转为 void**，然后将 dlsym 返回的 void* 赋值给它解引用后的变量（即 func）
    // (2) func = (void (*)())dlsym(handle, "your_function");
    // (3) func = reinterpret_cast<func_t>(dlsym(handle, "your_function"));
    // (4) 如下
    func = (func_t)dlsym(handle, "your_function");
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    // 调用符号
    func();

    // 关闭 .so 文件
    dlclose(handle);
    return 0;
}
