#!/bin/bash

SCRIPTS_DIR=$(dirname $(readlink -f "$0"))

# 引入参数解析库
source "$SCRIPTS_DIR/lib_arg_parser.sh" || { echo "Error: Failed to source lib_arg_parser.sh"; exit 1; }

# 定义必须参数和可选参数
required_args=("inference_type")
optional_args=("ort_provider" "trt_version" "cuda_version")

# 定义参数缩写
declare -A arg_short_map=(
    [inference_type]=it
    [ort_provider]=op
    [trt_version]=tv
    [cuda_version]=cv
)

# 定义每个参数的可选值列表（可选参数的第一个值为默认值）
inference_type_vals=(
    "mnn" "openvino" "onnxruntime" "tensorrt"
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


PROJECT_DIR=$(realpath "$SCRIPTS_DIR/../..")

export LD_LIBRARY_PATH="$PROJECT_DIR/build/target/Linux/lib:$PROJECT_DIR/third_party/opencv/Linux/x86_64/lib/Release:$PROJECT_DIR/third_party/RSLog/Linux/lib"
if [[ "$inference_type" == "mnn" ]]; then
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PROJECT_DIR/third_party/mnn/Linux/x86_64/lib"
elif [[ "$inference_type" == "openvino" ]]; then
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PROJECT_DIR/third_party/openvino/Linux/x86_64/runtime/lib/intel64"
elif [[ "$inference_type" == "onnxruntime" ]]; then
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PROJECT_DIR/third_party/onnxruntime/$ort_provider/Linux/x86_64/lib/Release"
    if [[ "$ort_provider" == "cuda" ]]; then
        export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PROJECT_DIR/third_party/cudnn/8.9-12.x/Linux/x86_64/lib/Release"
    fi
elif [[ "$inference_type" == "tensorrt" ]]; then
    if [[ "$trt_version" == "7" ]]; then
        if [[ "$cuda_version" == "12" ]]; then
            echo "TensorRT versoin 7 and CUDA version 12 is not supported."
            exit 1
        fi
        export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PROJECT_DIR/third_party/tensorrt/$trt_version/Linux/x86_64/lib/Release:$PROJECT_DIR/third_party/cudnn/8.1-11.x/Linux/x86_64/lib/Release:/usr/local/cuda-11.0/lib64"
    elif [[ "$trt_version" == "8" ]]; then
        if [[ "$cuda_version" == "11" ]]; then
            export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PROJECT_DIR/third_party/tensorrt/$trt_version-11.x/Linux/x86_64/lib/Release:$PROJECT_DIR/third_party/cudnn/8.1-11.x/Linux/x86_64/lib/Release"
        elif [[ "$cuda_version" == "12" ]]; then
            if [[ "$cuda_version" == "11" ]]; then
                echo "TensorRT versoin 10 and CUDA version 11 is not supported."
                exit 1
            fi
            export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PROJECT_DIR/third_party/tensorrt/$trt_version-12.x/Linux/x86_64/lib/Release:$PROJECT_DIR/third_party/cudnn/8.9-12.x/Linux/x86_64/lib/Release"
        fi
    else
        export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PROJECT_DIR/third_party/tensorrt/$trt_version/Linux/x86_64/lib/Release"
    fi
fi
mkdir -p $PROJECT_DIR/output/$inference_type
$PROJECT_DIR/build/target/Linux/bin/image_classifier_sample images "$PROJECT_DIR/model/breast_thyroid/rsm/linux/checkpoint-best-${inference_type}.rsm" $inference_type