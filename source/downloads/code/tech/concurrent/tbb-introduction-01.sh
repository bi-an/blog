#!/bin/bash
echo "==== 检查 Intel TBB (oneTBB) 安装情况 ===="

# 1. 包管理器检查
if command -v dpkg >/dev/null 2>&1; then
    echo "[dpkg] 检查:"
    dpkg -l | grep tbb || echo "  未找到 tbb 包 (Debian/Ubuntu)"
elif command -v rpm >/dev/null 2>&1; then
    echo "[rpm] 检查:"
    rpm -qa | grep tbb || echo "  未找到 tbb 包 (CentOS/RHEL)"
fi

# 2. pkg-config 检查
echo "[pkg-config] 检查:"
if command -v pkg-config >/dev/null 2>&1; then
    if pkg-config --exists tbb; then
        echo "  版本: $(pkg-config --modversion tbb)"
        echo "  CFLAGS: $(pkg-config --cflags tbb)"
        echo "  LIBS: $(pkg-config --libs tbb)"
    else
        echo "  未检测到 pkg-config 中的 tbb"
    fi
else
    echo "  系统未安装 pkg-config"
fi

# 3. 文件检查
echo "[文件] 检查:"
HEADER_PATH=$(find /usr/include /usr/local/include -type d -name tbb 2>/dev/null | head -n 1)
LIB_PATH=$(find /usr/lib /usr/local/lib /usr/lib64 -name "libtbb.*" 2>/dev/null | head -n 1)

if [ -n "$HEADER_PATH" ]; then
    echo "  头文件目录: $HEADER_PATH"
else
    echo "  未找到 tbb 头文件"
fi

if [ -n "$LIB_PATH" ]; then
    echo "  库文件: $LIB_PATH"
else
    echo "  未找到 tbb 库文件"
fi

# 4. 测试编译
echo "[编译测试] 尝试编译一个 TBB 示例..."
TMP_CODE=$(mktemp /tmp/test_tbb.XXXXXX.cpp)
cat > $TMP_CODE <<'EOF'
#include <tbb/tbb.h>
#include <iostream>
int main() {
    tbb::parallel_for(0, 5, [](int i){ std::cout << i << " "; });
    std::cout << std::endl;
    return 0;
}
EOF

g++ $TMP_CODE -o /tmp/test_tbb -ltbb -std=c++17 2>/tmp/tbb_err.log
if [ $? -eq 0 ]; then
    echo "  ✅ 编译成功，可以使用 TBB"
    /tmp/test_tbb
else
    echo "  ❌ 编译失败，错误信息如下："
    cat /tmp/tbb_err.log
fi

rm -f $TMP_CODE /tmp/test_tbb /tmp/tbb_err.log
