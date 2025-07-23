#include <iostream>

extern "C" {

// 一个简单的函数，供 main.cpp 测试
void your_function_name() {
    std::cout << "Hello from your_function_name in the shared library!" << std::endl;
}

}