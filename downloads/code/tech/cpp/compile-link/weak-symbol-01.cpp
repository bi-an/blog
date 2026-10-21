// 库侧：提供弱定义的默认实现
#include <cstdio>

extern "C" __attribute__((weak)) void on_event() {
    std::puts("[default] on_event");
}

extern "C" void run() {
    on_event();
}
