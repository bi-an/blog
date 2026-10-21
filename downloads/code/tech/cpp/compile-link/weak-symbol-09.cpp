// 弱引用：符号可能不存在，运行时判空
#include <cstdio>

extern "C" void optional_feature() __attribute__((weak));

int main() {
    if (optional_feature) {
        optional_feature();
    } else {
        std::puts("optional_feature not linked, skip");
    }
}
