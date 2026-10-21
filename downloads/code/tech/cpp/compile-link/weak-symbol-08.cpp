// 定义一个同名的强符号（普通函数默认是强符号）
extern "C" void optional_feature() {
    std::cout << "optional_feature implementation!" << std::endl;
}