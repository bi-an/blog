#include <dlfcn.h>

#include <iostream>

typedef void (*HelloFunc)();

int main() {
    // 打开共享库
    void* handle = dlopen("./libhello.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "dlopen failed: " << dlerror() << std::endl;
        return 1;
    }

    // 清除之前的错误
    dlerror();

    // 获取函数指针
    HelloFunc hello = (HelloFunc)dlsym(handle, "hello");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "dlsym failed: " << dlsym_error << std::endl;
        dlclose(handle);
        return 1;
    }

    // 打印函数地址
    std::cout << "Address of hello function: " << reinterpret_cast<void*>(hello) << std::endl;

    // 调用函数
    hello();

    // 关闭共享库
    dlclose(handle);
    return 0;
}