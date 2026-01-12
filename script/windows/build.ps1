# 获取脚本所在目录
$ScriptDir = Split-Path $MyInvocation.MyCommand.Path -Parent
. "$ScriptDir\lib_arg_parser.ps1"

# 定义必须参数和可选参数
$required_args = @("inference_type")
$optional_args = @(
    "msvc_version"
    "build_type",
    "build_shared",
    "enable_sample",
    "enable_test",
    "enable_tools",
    "ort_provider",
    "trt_version",
    "cuda_version"
)

# 参数缩写映射
$arg_short_map = @{
    inference_type = "it"
    msvc_version   = "mv"
    build_type     = "bt"
    build_shared   = "bs"
    enable_sample  = "es"
    enable_test    = "et"
    enable_tools   = "etools"
    ort_provider   = "op"
    trt_version    = "tv"
    cuda_version   = "cv"
}

# 各参数的可选值（第一个为默认值）
$inference_type_vals = @("mnn", "openvino", "onnxruntime", "tensorrt")
$msvc_version_vals   = @("2022", "2019")
$build_type_vals     = @("Release", "Debug")
$build_shared_vals   = @("ON", "OFF")
$enable_sample_vals  = @("ON", "OFF")
$enable_test_vals    = @("OFF", "ON")
$enable_tools_vals   = @("ON", "OFF")
$ort_provider_vals   = @("cpu", "cuda")
$trt_version_vals    = @("10", "8", "7")
$cuda_version_vals   = @("12", "11")

# 解析参数
ParseArgs $args

# 打印解析结果（用于调试）
PrintArgs

# 设置错误处理：任意命令失败则退出
$ErrorActionPreference = "Stop"

# 定义项目根目录
$PROJECT_ROOT = Join-Path $ScriptDir "..\.."
$PROJECT_ROOT = Resolve-Path $PROJECT_ROOT
Write-Host "Project root: $PROJECT_ROOT"

# 构建目录
$BUILD_DIR = Join-Path $PROJECT_ROOT "build"
if (Test-Path $BUILD_DIR) {
    Remove-Item -Path $BUILD_DIR -Recurse
}
New-Item -ItemType Directory -Path $BUILD_DIR -Force | Out-Null

# 进入 build 目录
Set-Location $BUILD_DIR

Write-Host "Configuring rayshape deploy for Windows ..."

# 初始化 CMake 变量
$enable_mnn            = "OFF"
$enable_openvino       = "OFF"
$enable_onnxruntime    = "OFF"
$enable_tensorrt       = "OFF"
$enable_cuda_device    = "OFF"
$rs_cuda_version       = ""

switch ($inference_type) {
    "mnn" {
        $enable_mnn = "ON"
    }
    "openvino" {
        $enable_openvino = "ON"
    }
    "onnxruntime" {
        $enable_onnxruntime = "ON"
        if ($ort_provider -eq "cuda") {
            $enable_cuda_device = "ON"
        }
    }
    "tensorrt" {
        $enable_tensorrt = "ON"
        $enable_cuda_device = "ON"
    }
    default {
        Write-Error "Invalid inference_type: $inference_type"
        exit 1
    }
}

# 设置 CUDA 版本
switch ($cuda_version) {
    "12" { $rs_cuda_version = "12.0" }
    "11" { $rs_cuda_version = "11.0" }
    default {
        Write-Error "Invalid cuda_version: $cuda_version"
        exit 1
    }
}

# 默认 build_type 为 Release
if (-not $build_type) { $build_type = "Release" }

# 构建 CMake 命令行参数
$cmakeArgs = @(
    "..",
    "-DCMAKE_BUILD_TYPE=$build_type",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    "-DCMAKE_C_COMPILER=/usr/bin/gcc",
    "-DCMAKE_CXX_COMPILER=/usr/bin/g++",
    "-DCMAKE_SYSTEM_NAME=Windows",
    "-DCMAKE_SYSTEM_PROCESSOR=$env:PROCESSOR_ARCHITECTURE",
    "-DENABLE_BUILD_SHARED=$build_shared",
    "-DENABLE_SYMBOL_HIDE=OFF",
    "-DENABLE_COVERAGE=OFF",
    "-DENABLE_STATIC_RUNTIME=ON",
    "-DENABLE_GLIBCXX_USE_CXX14_ABI=ON",
    "-DENABLE_TIME_PROFILER=OFF",
    "-DENABLE_RAPIDJSON=ON",
    "-DENABLE_CEREAL=ON",
    "-DENABLE_BASE=ON",
    "-DENABLE_DEVICE=ON",
    "-DENABLE_CPU_DEVICE=ON",
    "-DENABLE_INFERENCE=ON",
    "-DENABLE_KERNEL=ON",
    "-DENABLE_SAMPLE=$enable_sample",
    "-DENABLE_TEST=$enable_test",
    "-DENABLE_TOOLS=$enable_tools",
    "-DENABLE_3RD_OPENCV=ON",
    "-DENABLE_MNN_INFERENCE=$enable_mnn",
    "-DENABLE_3RD_MNN=$enable_mnn",
    "-DENABLE_OPENVINO_INFERENCE=$enable_openvino",
    "-DENABLE_3RD_OPENVINO=$enable_openvino",
    "-DENABLE_ONNXRUNTIME_INFERENCE=$enable_onnxruntime",
    "-DENABLE_3RD_ONNXRUNTIME=$enable_onnxruntime",
    "-DRS_ONNXRUNTIME_PROVIDER=$ort_provider",
    "-DENABLE_TENSORRT_INFERENCE=$enable_tensorrt",
    "-DENABLE_3RD_TENSORRT=$enable_tensorrt",
    "-DRS_TENSORRT_VERSION=$trt_version",
    "-DRS_CUDA_VERSION=$rs_cuda_version",
    "-DENABLE_CUDA_DEVICE=$enable_cuda_device"
)

Write-Host "Running CMake configure..." -ForegroundColor Cyan
Write-Host "CMake command: cmake $cmakeArgs"

if ([string]::IsNullOrEmpty($env:ARCH)) {
    # 检测当前进程和系统架构
    $processorArch = $env:PROCESSOR_ARCHITECTURE
    $wow64Arch = $env:PROCESSOR_ARCHITEW6432

    if ($processorArch -eq "AMD64" -or $wow64Arch -eq "AMD64") {
        $env:ARCH = "x64"
    } else {
        $env:ARCH = "x86"
    }
}

# 执行 CMake 配置
$cmake_msvc_version = ""
switch ($msvc_version) {
    "2022" {
        $cmake_msvc_version = "Visual Studio 17 2022"
    }
    "2019" {
        $cmake_msvc_version = "Visual Studio 16 2019"
    }
    default {
        Write-Error "Invalid msvc_version: $msvc_version"
        exit 1
    }
}

try {
    cmake -G "$cmake_msvc_version" -A "$env:ARCH" @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed with exit code $LASTEXITCODE"
    }
}
catch {
    Write-Error "[ERROR] CMake configuration failed: $_"
    exit 1
}

# 构建项目
Write-Host "Building project with configuration: $BUILD_TYPE..." -ForegroundColor Cyan
try {
    cmake --build . --config $BUILD_TYPE
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
}
catch {
    Write-Error "[ERROR] Build failed: $_"
    exit 1
}

# 成功完成
Write-Host "Build completed successfully." -ForegroundColor Green
Write-Host "Output directory: $BUILD_DIR/target/Linux/"

Set-Location $PROJECT_ROOT