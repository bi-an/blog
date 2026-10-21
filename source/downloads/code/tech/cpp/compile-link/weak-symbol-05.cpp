// plugin_checker.cpp：弱引用，检测可选模块是否存在
#include <iostream>

// 声明一个外部的弱符号函数（它可能在其他目标文件中，也可能不存在）
extern "C" __attribute__((weak)) void optional_plugin_init();

int main() {
    std::cout << "Checking for optional plugin..." << std::endl;

    // 检查弱符号地址是否为 nullptr
    if (optional_plugin_init != nullptr) {
        std::cout << "Plugin found! Initializing..." << std::endl;
        optional_plugin_init();
    } else {
        std::cout << "Plugin not found. Skipping optional feature." << std::endl;
    }

    return 0;
}
