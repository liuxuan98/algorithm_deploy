#!/bin/bash

# 编译 rayshape deploy for linux with tools
set -e

# 定义项目根目录
PROJECT_ROOT=$(cd "$(dirname "$0")/../.." && pwd)
echo "Project root: $PROJECT_ROOT"

# 创建 build 文件夹（如果不存在）
BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"

# 进入 build 文件夹
cd "$BUILD_DIR" || { echo "无法进入 build 目录"; exit 1; }

echo "Configuring rayshape deploy for linux with tools..."

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
    -DENABLE_STATIC_RUNTIME=ON \
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
    -DENABLE_SAMPLE=OFF \
    -DENABLE_TEST=OFF \
    -DENABLE_TOOLS=ON \
    -DENABLE_PACK_MODELS_TOOL=ON \
    -DENABLE_3RD_MNN=ON \
    -DENABLE_3RD_OPENCV=ON \
    -DENABLE_3RD_OPENVINO=ON

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

echo "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build completed successfully!"
echo "Compile commands database generated at: $BUILD_DIR/compile_commands.json"
echo ""
echo "Tools built:"
echo "  - pack_model_tool: $BUILD_DIR/build/target/Linux/lib/pack_model_tool" 