#!/bin/bash

SCRIPTS_DIR=$(dirname $(readlink -f "$0"))

# 引入参数解析库
source "$SCRIPTS_DIR/lib_arg_parser.sh" || { echo "Error: Failed to source lib_arg_parser.sh"; exit 1; }

# 定义必须参数和可选参数
required_args=("inference_type")
optional_args=(
    "build_type" "build_shared"
    "enable_sample" "enable_test" "enable_tools"
    "ort_provider" "trt_version" "cuda_version"
)

# 定义参数缩写
declare -A arg_short_map=(
    [inference_type]=it
    [build_type]=bt
    [build_shared]=bs
    [enable_sample]=es
    [enable_test]=et
    [enable_tools]=etools
    [ort_provider]=op
    [trt_version]=tv
    [cuda_version]=cv
)

# 定义每个参数的可选值列表（可选参数的第一个值为默认值）
inference_type_vals=(
    "mnn" "openvino" "onnxruntime" "tensorrt"
)
build_type_vals=(
    "Release" "Debug"
)
build_shared_vals=(
    "ON" "OFF"
)
enable_sample_vals=(
    "ON" "OFF"
)
enable_test_vals=(
    "OFF" "ON"
)
enable_tools_vals=(
    "ON" "OFF"
)
ort_provider_vals=(
    "cpu" "cuda"
)
trt_version_vals=(
    "10" "8" "7"
)
cuda_version_vals=(
    "12" "11"
)

# 调用解析函数
ParseArgs "$@"

# 输出解析结果
PrintArgs


# 编译 rayshape deploy for linux (Release build for production)
set -e

# 定义项目根目录
PROJECT_ROOT=$(cd "$(dirname "$0")/../.." && pwd)
echo "Project root: $PROJECT_ROOT"

# 创建 build 文件夹（如果不存在）
BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"

# 进入 build 文件夹
cd "$BUILD_DIR" || { echo "无法进入 build 目录"; exit 1; }

echo "Configuring rayshape deploy for linux (Release)..."


enable_mnn="OFF"
enable_openvino="OFF"
enable_onnxruntime="OFF"
enable_tensorrt="OFF"
enable_cuda_device="OFF"
if [[ "$inference_type" == "mnn" ]]; then
    enable_mnn="ON"
elif [[ "$inference_type" == "openvino" ]]; then
    enable_openvino="ON"
elif [[ "$inference_type" == "onnxruntime" ]]; then
    enable_onnxruntime="ON"
    if [[ "$ort_provider" == "cuda" ]]; then
        enable_cuda_device="ON"
    fi
elif [[ "$inference_type" == "tensorrt" ]]; then
    enable_tensorrt="ON"
    enable_cuda_device="ON"
fi
if [[ "$cuda_version" == "12" ]]; then
    rs_cuda_version="12.0"
elif [[ "$cuda_version" == "11" ]]; then
    rs_cuda_version="11.0"
fi

# 执行 CMake 配置
cmake .. \
    -DCMAKE_BUILD_TYPE=$build_type \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_C_COMPILER=/usr/bin/gcc \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DENABLE_BUILD_SHARED=$build_shared \
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
    -DENABLE_KERNEL=ON \
    -DENABLE_SAMPLE=$enable_sample \
    -DENABLE_TEST=$enable_test \
    -DENABLE_TOOLS=$enable_tools \
    -DENABLE_3RD_OPENCV=ON \
    -DENABLE_MNN_INFERENCE=$enable_mnn \
    -DENABLE_3RD_MNN=$enable_mnn \
    -DENABLE_OPENVINO_INFERENCE=$enable_openvino \
    -DENABLE_3RD_OPENVINO=$enable_openvino \
    -DENABLE_ONNXRUNTIME_INFERENCE=$enable_onnxruntime \
    -DENABLE_3RD_ONNXRUNTIME=$enable_onnxruntime \
    -DRS_ONNXRUNTIME_PROVIDER=$ort_provider \
    -DENABLE_TENSORRT_INFERENCE=$enable_tensorrt \
    -DENABLE_3RD_TENSORRT=$enable_tensorrt \
    -DRS_TENSORRT_VERSION=$trt_version \
    -DRS_CUDA_VERSION=$rs_cuda_version \
    -DENABLE_CUDA_DEVICE=$enable_cuda_device

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

echo "Release build completed successfully!"
echo "Output directory: $BUILD_DIR/target/Linux/"