#!/bin/bash

# 仅配置项目用于IDE支持，不进行完整构建
set -e

# 定义项目根目录
PROJECT_ROOT=$(cd "$(dirname "$0")/../.." && pwd)
echo "Project root: $PROJECT_ROOT"

# 创建 build 文件夹（如果不存在）
BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"

# 进入 build 文件夹
cd "$BUILD_DIR" || { echo "无法进入 build 目录"; exit 1; }

echo "Configuring rayshape deploy for IDE support..."

# 执行 CMake 配置
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_C_COMPILER=/usr/bin/gcc \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
    -DENABLE_BUILD_SHARED=ON \
    -DENABLE_SYMBOL_HIDE=OFF \
    -DENABLE_COVERAGE=OFF \
    -DENABLE_GLIBCXX_USE_CXX14_ABI=ON \
    -DENABLE_TIME_PROFILER=OFF \
    -DENABLE_RAPIDJSON=ON \
    -DENABLE_CEREAL=ON \
    -DENABLE_BASE=ON \
    -DENABLE_DEVICE=ON \
    -DENABLE_CPU_DEVICE=ON \
    -DENABLE_INFERENCE=ON \
    -DENABLE_MNN_INFERENCE=ON \
    -DENABLE_OPENVINO_INFERENCE=OFF \
    -DENABLE_KERNEL=ON \
    -DENABLE_SAMPLE=ON \
    -DENABLE_TEST=OFF \
    -DENABLE_3RD_MNN=ON \
    -DENABLE_3RD_OPENCV=ON \
    -DENABLE_3RD_OPENVINO=ON

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

echo "Configuration completed successfully!"
echo "Compile commands database generated at: $BUILD_DIR/compile_commands.json"
echo ""
echo "现在您可以："
echo "1. 重新加载 VSCode 窗口 (Ctrl+Shift+P -> 'Developer: Reload Window')"
echo "2. 或者重启 VSCode"
echo "3. 红色波浪线应该会消失" 