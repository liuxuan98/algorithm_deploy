#!/bin/bash

SCRIPTS_DIR=$(dirname $(readlink -f "$0"))

# 引入参数解析库
source "$SCRIPTS_DIR/lib_arg_parser.sh" || { echo "Error: Failed to source lib_arg_parser.sh"; exit 1; }

# 定义必须参数和可选参数
required_args=("model_type" "inference_type")
optional_args=("encrypt" "ort_provider")

# 定义参数缩写
declare -A arg_short_map=(
    [model_type]=mt
    [inference_type]=it
    [encrypt]=e
    [ort_provider]=op
)

# 定义每个参数的可选值列表（可选参数的第一个值为默认值）
model_type_vals=(
    "mnn" "openvino" "onnx"
)
inference_type_vals=(
    "mnn" "openvino" "onnxruntime" "tensorrt"
)
encrypt_vals=(
    "on" "off"
)
ort_provider_vals=(
    "cpu" "cuda"
)

# 调用解析函数
ParseArgs "$@"

# 输出解析结果
PrintArgs


# 定义模型后缀
declare -A model_suffix_map=(
    ["mnn"]="mnn"
    ["onnx"]="onnx"
)

PROJECT_DIR=$(realpath "$SCRIPTS_DIR/../..")

pack_models_args="serialize -t $model_type"
if [[ "$model_type" == "openvino" ]]; then
    pack_models_args="${pack_models_args} --xml $PROJECT_DIR/model/breast_thyroid/$model_type/checkpoint-best.xml --bin $PROJECT_DIR/model/breast_thyroid/$model_type/checkpoint-best.bin"
else
    pack_models_args="${pack_models_args} -i $PROJECT_DIR/model/breast_thyroid/$model_type/checkpoint-best.${model_suffix_map[$model_type]}"
fi
pack_models_args="${pack_models_args} --config $PROJECT_DIR/model/breast_thyroid/cfg/cfg_${inference_type}.json -o $PROJECT_DIR/model/breast_thyroid/rsm/linux/checkpoint-best-${inference_type}.rsm"
if [[ "$encrypt" == "ON" ]]; then
    pack_models_args="${pack_models_args} --encrypt"
fi

export LD_LIBRARY_PATH="$PROJECT_DIR/build/target/Linux/lib:$PROJECT_DIR/third_party/opencv/Linux/x86_64/lib/Release:$PROJECT_DIR/third_party/RSLog/Linux/lib"
mkdir -p $PROJECT_DIR/model/breast_thyroid/rsm
$PROJECT_DIR/build/bin/pack_models $pack_models_args