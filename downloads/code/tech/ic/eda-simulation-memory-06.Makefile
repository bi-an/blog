
# 编译器配置
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O3 -pthread
LDFLAGS := -pthread

# 项目结构
SRC_DIR := .
BUILD_DIR := build
TARGET := $(BUILD_DIR)/cache_simulator

# 源文件列表
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# 默认目标
all: $(BUILD_DIR) $(TARGET)

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 主目标链接
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

# 编译规则
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# 包含依赖关系
-include $(DEPS)

# 清理
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
