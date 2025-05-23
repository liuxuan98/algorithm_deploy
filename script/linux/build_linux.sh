# 编译 rayshape deploy for linux todo........
#!/bin/bash

# 定义项目根目录
PROJECT_ROOT=$(pwd)

# 创建 build_linux 文件夹（如果不存在）
mkdir -p "$PROJECT_ROOT/build_linux"

# 进入 build_linux 文件夹
cd "$PROJECT_ROOT/build_linux" || { echo "无法进入 build_linux 目录"; exit 1; }

# 执行构建操作（根据你的需求修改这里的命令）
echo "Configuring rayshape deploy for linux ..."

# 示例：调用 CMake 配置和构建
# cmake .. \
    -DCMAKE_SYSTEM_NAME=Linux\
    -DRS_DEPLOY_BUILD_SHARED=ON\  # 构建动态库
    make -j7 # 构建
# cmake --build .

echo "Configure Done"


