CXX := g++
CXXFLAGS := -O2 -Wall -std=c++17

# TBB 路径 (根据需要修改，若系统已安装 libtbb-dev 可省略)
TBB_INC := /usr/include
TBB_LIB := /usr/lib
CXXFLAGS += -I$(TBB_INC)
LDFLAGS := -L$(TBB_LIB) -ltbb

SRC_DIR := .
BUILD_DIR := build

# 源文件列表
SRCS := 1_message_queue.cpp 2_flow_graph.cpp 3_pipeline.cpp

# 生成的目标 (去掉 .cpp 后缀，加 build/ 前缀)
TARGETS := $(SRCS:%.cpp=$(BUILD_DIR)/%)

# 对应的 .o 文件
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

# 添加头文件依赖
DEPS := tasks.hpp

all: $(TARGETS)

$(BUILD_DIR):
	mkdir -p $@

# 规则：生成可执行文件
$(BUILD_DIR)/%: $(BUILD_DIR)/%.o | $(BUILD_DIR)
	$(CXX) $< -o $@ $(LDFLAGS)

# 规则：编译 .cpp -> .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
