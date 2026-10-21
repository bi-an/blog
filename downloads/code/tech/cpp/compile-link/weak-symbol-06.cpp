// plugin_impl.cpp：提供 optional_plugin_init 的强定义
#include <iostream>

extern "C" void optional_plugin_init() {
    std::cout << "[Plugin] initialized." << std::endl;
}
