#!/bin/bash

# lib_arg_parser.sh - 通用参数解析库

# 使用示例：
# test.sh - 使用通用参数解析库进行输入参数解析
#     SCRIPTS_DIR=$(dirname $(readlink -f "$0"))
#
#     # 引入参数解析库
#     source "$SCRIPTS_DIR/lib_arg_parser.sh" || { echo "Error: Failed to source lib_arg_parser.sh"; exit 1; }
#
#     # 定义必须参数和可选参数
#     required_args=("test_name")
#     optional_args=("verbose")
#
#     # 定义参数缩写（可选）
#     declare -A arg_short_map=(
#         [test_name]=n
#         [verbose]=v
#     )
#
#     # 定义每个参数的可选值列表（可选参数的第一个值为默认值）
#     test_name_vals=("flycv" "ppl.cv")
#     verbose_vals=("off" "on")
#
#     # 调用解析函数
#     ParseArgs "$@"
#
#     # 输出解析结果
#     PrintArgs


# 工具函数：判断值是否在数组中（忽略大小写）
Contains() {
    local value=$(echo "$1" | tr '[:upper:]' '[:lower:]')
    shift

    # 如果没有剩余参数（即数组为空），返回 0
    if [ $# -eq 0 ]; then
        return 0
    fi

    for item in "$@"; do
        if [[ "$(echo "$item" | tr '[:upper:]' '[:lower:]')" == "$value" ]]; then
            return 0
        fi
    done
    return 1
}

# 参数校验函数：检查 required_args 是否都传入且合法，检查 optional_args 是否合法
CheckArgs() {
    for arg in "${required_args[@]}"; do
        local val
        eval 'val="${'"$arg"'}"'

        if [ -z "$val" ]; then
            echo "Error: Missing required argument '--$arg'"
            Usage
        fi

        eval 'vals=("${'"$arg"'_vals[@]}")'
        Contains "$val" "${vals[@]}" || {
            echo "Error: Invalid value for $arg: $val"
            echo "Allowed values: ${vals[*]}"
            Usage
        }
    done

    for arg in "${optional_args[@]}"; do
        local val
        eval 'val="${'"$arg"'}"'

        if [ -n "$val" ]; then
            eval 'vals=("${'"$arg"'_vals[@]}")'
            Contains "$val" "${vals[@]}" || {
                echo "Error: Invalid value for $arg: $val"
                echo "Allowed values: ${vals[*]}"
                Usage
            }
        fi
    done
}

# Usage 函数：生成帮助信息
Usage() {
    echo "Usage: $0"

    # 添加 --help 的帮助信息
    echo "  --help,-h  # display help information"

    # 输出其他参数的帮助信息
    for arg in "${required_args[@]}"; do
        short=""
        if [[ -n "${arg_short_map[$arg]:-}" ]]; then
            short=",-${arg_short_map[$arg]}"
        fi
        eval 'vals=("${'"$arg"'_vals[@]}")'
        if [[ ${#vals[@]} -gt 0 ]]; then
            echo "  --$arg$short <${arg}>  # allowed values: ${vals[*]}"
        else
            echo "  --$arg$short <${arg}>"
        fi
    done
    for arg in "${optional_args[@]}"; do
        short=""
        if [[ -n "${arg_short_map[$arg]:-}" ]]; then
            short=",-${arg_short_map[$arg]}"
        fi
        eval 'vals=("${'"$arg"'_vals[@]}")'
        if [[ ${#vals[@]} -gt 0 ]]; then
            echo "  [--$arg$short <${arg}>]  # allowed values: ${vals[*]}"
        else
            echo "  [--$arg$short <${arg}>]"
        fi
    done

    # 判断是否传入了退出码
    if [[ -n "$1" && "$1" =~ ^[0-9]+$ ]]; then
        exit "$1"
    else
        exit 1
    fi
}

# 构建缩写到长参数名的映射表
BuildShortMap() {
    unset short_to_long
    declare -gA short_to_long=()

    for arg in "${required_args[@]}" "${optional_args[@]}"; do
        short=${arg_short_map[$arg]:-}
        if [ -n "$short" ]; then
            short_to_long["-$short"]="--$arg"
        fi
    done
}

# 填充未设置可选参数的默认值：使用其 vals 数组的第一个值
ApplyDefaults() {
    for arg in "${optional_args[@]}"; do
        local val
        eval 'val="${'"$arg"'}"'

        if [ -z "$val" ]; then
            # 取该参数的可选值列表的第一个值作为默认值
            eval 'vals=("${'"$arg"'_vals[@]}")'
            if [ ${#vals[@]} -gt 0 ]; then
                eval "$arg"="\"${vals[0]}\""
            fi
        fi
    done
}

# 解析命令行参数（根据 required_args 和 optional_args 处理）
ParseArgs() {
    provided_args=()

    BuildShortMap

    while [ $# -gt 0 ]; do
        case "$1" in
            --help|-h)
                Usage 0
                ;;
        esac

        local matched_short=""
        # 查找是否匹配到缩写
        for key in "${!short_to_long[@]}"; do
            if [[ "$1" == "$key" ]]; then
                matched_short="$key"
                set -- "${short_to_long[$key]}" "${@:2}"
                break
            fi
        done

        matched=0
        for arg in "${required_args[@]}" "${optional_args[@]}"; do
            if [ "$1" == "--$arg" ]; then
                if [ $# -lt 2 ]; then
                    local display_arg="--$arg"
                    if [[ -n "$matched_short" ]]; then
                        display_arg="$matched_short"
                    fi
                    echo "Error: Missing value after $display_arg"
                    Usage 1
                fi
                eval "$arg"="\"$2\""
                provided_args+=("$arg")
                shift 2
                matched=1
                break
            fi
        done

        if [ $matched -eq 0 ]; then
            echo "Unknown option: $1"
            Usage 1
        fi
    done

    CheckArgs

    ApplyDefaults
}

PrintArgs() {
    echo "--- args:"

    # 输出解析结果：只输出那些被命令行传入的参数
    for arg in "${provided_args[@]}"; do
        eval 'val="${'"$arg"'}"'
        echo "  $arg: $val"
    done

    # 输出默认值参数（未提供的 optional 参数）
    for arg in "${optional_args[@]}"; do
        if ! [[ " ${provided_args[@]} " =~ " ${arg} " ]]; then
            eval 'val="${'"$arg"'}"'
            echo "  $arg: $val (default)"
        fi
    done

    echo "---"
}