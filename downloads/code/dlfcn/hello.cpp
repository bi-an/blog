#include <iostream>

extern "C" void hello() {
    std::cout << "Hello from shared library!" << std::endl;
}