# 设置项目名称
cmake_minimum_required(VERSION 3.10)
project(MyProject)

# 设置默认构建类型为 Debug
set(CMAKE_BUILD_TYPE Debug)

# 生成动态库
add_library(mylib SHARED mylib.c)
add_library(fake_dlfcn SHARED fake_dlfcn.c)

# 添加可执行文件
add_executable(main main.c)

# 链接动态库到主程序
target_link_libraries(main PRIVATE mylib dl fake_dlfcn)