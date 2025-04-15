#include <iostream>

#include "fake_dlfcn.cpp"  // 确保路径正确，或者将其改为头文件引用

// #include <dlfcn.h>
// #include <iostream>

// void preloadLibrary(const char *libraryPath) {
//     void *handle = dlopen(libraryPath, RTLD_LAZY);
//     if (!handle) {
//         std::cerr << "Failed to preload library: " << libraryPath << std::endl;
//         std::cerr << "Error: " << dlerror() << std::endl;
//         return;
//     }
//     std::cout << "Library preloaded successfully: " << libraryPath << std::endl;

//     // 注意：不要调用 dlclose(handle)，否则会卸载库并从 /proc/self/maps 中移除
// }

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>

void *mapLibrary(const char *libraryPath) {
    int fd = open(libraryPath, O_RDONLY);
    if (fd < 0) {
        std::cerr << "Failed to open library: " << libraryPath << std::endl;
        return nullptr;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size <= 0) {
        std::cerr << "Failed to get library size: " << libraryPath << std::endl;
        close(fd);
        return nullptr;
    }

    void *mapped = mmap(nullptr, size, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
    close(fd);

    if (mapped == MAP_FAILED) {
        std::cerr << "Failed to map library: " << libraryPath << std::endl;
        return nullptr;
    }

    std::cout << "Library mapped successfully: " << libraryPath << std::endl;
    return mapped;
}

int main() {
    const char *libraryPath = "./libtestlib.so";    // 替换为实际的共享库路径
    const char *symbolName = "your_function_name";  // 替换为实际的符号名称

    // 提前导入so到/proc中
    void *mappedLibrary = mapLibrary(libraryPath);
    if (!mappedLibrary) {
        return 1;  // 映射失败
    }

    // 加载共享库
    void *handle = fake_dlopen(libraryPath, 0);
    if (!handle) {
        std::cerr << "Failed to load library: " << libraryPath << std::endl;
        return 1;
    }
    std::cout << "Library loaded successfully: " << libraryPath << std::endl;

    // 查找符号
    void *symbol = fake_dlsym(handle, symbolName);
    if (!symbol) {
        std::cerr << "Failed to find symbol: " << symbolName << std::endl;
        fake_dlclose(handle);
        return 1;
    }
    std::cout << "Symbol found: " << symbolName << " at address " << symbol << std::endl;

    // 调用符号（假设符号是一个函数）
    using FunctionType = void (*)();  // 根据符号的实际类型修改
    auto function = reinterpret_cast<FunctionType>(symbol);
    function();

    // 关闭共享库
    if (fake_dlclose(handle) == 0) {
        std::cout << "Library closed successfully." << std::endl;
    } else {
        std::cerr << "Failed to close library." << std::endl;
    }

    return 0;
}