// main_override.cpp：通过强符号重写弱符号
#include <iostream>

// 定义一个同名的强符号（普通函数默认是强符号）
// 链接器会用这个强符号覆盖 library.cpp 中的弱符号
extern "C" void user_custom_hook() {
    std::cout << "[Main] Overridden strong hook implementation!" << std::endl;
}

void execute_task();

int main() {
    execute_task();
    return 0;
}
