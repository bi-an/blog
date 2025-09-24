cmake_minimum_required(VERSION 3.10)

# 项目名称和C++标准
project(tbb_test CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找TBB库并链接到项目
find_package(TBB REQUIRED)

# 获取当前目录下的所有 .cpp 文件
file(GLOB CPP_FILES "*.cpp")

# 为每个 .cpp 文件创建一个可执行文件并链接到 TBB 库
foreach(CPP_FILE ${CPP_FILES})
    get_filename_component(EXECUTABLE_NAME ${CPP_FILE} NAME_WE)
    add_executable(${EXECUTABLE_NAME} ${CPP_FILE})
    # TBB::tbb表示一个由find_package(TBB REQUIRED)命令找到并导入的TBB库目标。
    # 使用这种方式，CMake会自动处理包含路径、库路径和其他编译选项。
    target_link_libraries(${EXECUTABLE_NAME} TBB::tbb)
endforeach()