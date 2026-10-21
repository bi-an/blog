// library.cpp：库文件，包含弱符号定义
#include <iostream>

// 使用 __attribute__((weak)) 将函数定义为弱符号
extern "C" __attribute__((weak)) void user_custom_hook() {
    std::cout << "[Library] Default weak hook implementation." << std::endl;
}

void execute_task() {
    std::cout << "[Library] Executing task..." << std::endl;
    user_custom_hook(); // 调用弱符号函数
}
