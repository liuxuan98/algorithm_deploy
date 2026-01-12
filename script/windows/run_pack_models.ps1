# 获取脚本所在目录
$ScriptDir = Split-Path $MyInvocation.MyCommand.Path -Parent
. "$ScriptDir\lib_arg_parser.ps1" -ErrorAction Stop

# 定义必须参数和可选参数
$required_args = @("model_type", "inference_type")
$optional_args = @("encrypt", "ort_provider", "trt_version", "cuda_version")

# 参数缩写映射
$arg_short_map = @{
    model_type     = "mt"
    inference_type = "it"
    encrypt        = "e"
    ort_provider   = "op"
    trt_version    = "tv"
    cuda_version   = "cv"
}

# 各参数的可选值（第一个为默认值）
$model_type_vals = @("mnn", "openvino", "onnx")
$inference_type_vals = @("mnn", "openvino", "onnxruntime", "tensorrt")
$encrypt_vals = @("on", "off")
$ort_provider_vals = @("cpu", "cuda")
$trt_version_vals = @("10", "8", "7")
$cuda_version_vals = @("12", "11")

# 解析参数
ParseArgs $args

# 输出解析结果（用于调试）
PrintArgs

# 检查必要变量是否已设置
if (-not $model_type) {
    Write-Error "Missing required argument: model_type"
    exit 1
}
if (-not $inference_type) {
    Write-Error "Missing required argument: inference_type"
    exit 1
}

# 定义模型后缀映射
$model_suffix_map = @{
    "mnn"    = "mnn"
    "onnx"   = "onnx"
    # openvino 不使用此映射
}

# 项目根目录
$PROJECT_DIR = (Resolve-Path (Join-Path $ScriptDir "..\..")).Path
Write-Host "Project directory: $PROJECT_DIR"

# 构建 pack_models 的参数列表
$pack_models_args = @("serialize", "-t", $model_type)

# 根据 model_type 添加输入路径
if ($model_type -eq "openvino") {
    $xmlPath = "$PROJECT_DIR\model\breast_thyroid\openvino\checkpoint-best.xml"
    $binPath = "$PROJECT_DIR\model\breast_thyroid\openvino\checkpoint-best.bin"

    if (-not (Test-Path $xmlPath)) {
        Write-Error "OpenVINO XML model not found: $xmlPath"
        exit 1
    }
    if (-not (Test-Path $binPath)) {
        Write-Error "OpenVINO BIN model not found: $binPath"
        exit 1
    }

    $pack_models_args += @("--xml", $xmlPath, "--bin", $binPath)
}
else {
    $suffix = $model_suffix_map[$model_type]
    if (-not $suffix) {
        Write-Error "Unsupported model_type for suffix: $model_type"
        exit 1
    }

    $inputPath = "$PROJECT_DIR\model\breast_thyroid\$model_type\checkpoint-best.$suffix"
    if (-not (Test-Path $inputPath)) {
        Write-Error "Model file not found: $inputPath"
        exit 1
    }

    $pack_models_args += @("-i", $inputPath)
}

# 添加通用参数
$configPath = "$PROJECT_DIR\model\breast_thyroid\cfg\cfg_${inference_type}.json"
if (-not (Test-Path $configPath)) {
    Write-Warning "Config file not found: $configPath (will still proceed)"
}
$pack_models_args += @("--config", $configPath)

$outputDir = "$PROJECT_DIR\model\breast_thyroid\rsm\windows"
$outputFile = "$outputDir\checkpoint-best-${inference_type}.rsm"
$pack_models_args += @("-o", $outputFile)

# 如果启用加密
if ($encrypt -ieq "on") {
    $pack_models_args += "--encrypt"
}

# 确保输出目录存在
New-Item -ItemType Directory -Path $outputDir -Force | Out-Null

# 设置 PATH（用于 Windows 环境下的库加载）
$dll_paths = @(
    "$PROJECT_DIR\build\target\Windows\bin\Release",
    "$PROJECT_DIR\third_party\RSLog\Windows\bin"
)
if ($inference_type -eq "mnn") {
    $dll_paths += "$PROJECT_DIR\third_party\mnn\Windows\AMD64\bin"
}
elseif ($inference_type -eq "openvino") {
    $dll_paths += "$PROJECT_DIR\third_party\openvino\Windows\AMD64\bin\intel64\Release"
    $dll_paths += "$PROJECT_DIR\third_party\openvino\Windows\AMD64\3rdparty\tbb\bin"
}
elseif($inference_type -eq "onnxruntime") {
    # 设置ONNXRuntime路径没有作用
    $dll_paths += "$PROJECT_DIR\third_party\onnxruntime\$ort_provider\Windows\AMD64\lib\Release"
    Copy-Item -Path "$PROJECT_DIR\third_party\onnxruntime\$ort_provider\Windows\AMD64\lib\Release\*.dll" -Destination "$PROJECT_DIR\build\bin\Release"
    if ($ort_provider -eq "cuda") {
        $dll_paths += "$PROJECT_DIR\third_party\cudnn\8.9-12.x\Windows\AMD64\bin\Release"
        $dll_paths += "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v${cuda_version}.0\bin"
    }
}
elseif($inference_type -eq "tensorrt") {
    if ($trt_version -eq "7") {
        if ($cuda_version -eq "12") {
            Write-Error "TensorRT versoin 7 and CUDA version 12 is not supported."
            exit 1
        }
        $dll_paths += @(
            "$PROJECT_DIR\third_party\tensorrt\7\Windows\AMD64\lib\Release",
            "$PROJECT_DIR\third_party\cudnn\8.1-11.x\Windows\AMD64\bin\Release"
        )
    }
    elseif ($trt_version -eq "8") {
        if ($cuda_version -eq "11") {
            $dll_paths += @(
                "$PROJECT_DIR\third_party\tensorrt\8-11.x\Windows\AMD64\lib\Release",
                "$PROJECT_DIR\third_party\cudnn\8.1-11.x\Windows\AMD64\bin\Release"
            )
        }
        elseif ($cuda_version -eq "12") {
            $dll_paths += @(
                "$PROJECT_DIR\third_party\tensorrt\8-12.x\Windows\AMD64\lib\Release",
                "$PROJECT_DIR\third_party\cudnn\8.9-12.x\Windows\AMD64\bin\Release"
            )
        }
        else {
            Write-Error "Unsupported cuda_version for TensorRT 8: $cuda_version"
            exit 1
        }
    }
    elseif ($trt_version -eq "10") {
        if ($cuda_version -eq "11") {
            Write-Error "TensorRT versoin 10 and CUDA version 11 is not supported."
            exit 1
        }
        $dll_paths += "$PROJECT_DIR\third_party\tensorrt\10\Windows\AMD64\lib\Release"
    }
    else {
        Write-Error "Unsupported trt_version: $trt_version, using generic path."
        exit 1
    }
    $dll_paths += "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v${cuda_version}.0\bin"
}
$env:PATH = ($dll_paths -join ';') + ";" + $env:PATH

Write-Host "PATH: $env:PATH" -ForegroundColor Cyan

# 执行 pack_models 命令
$Executable = "$PROJECT_DIR\build\bin\Release\pack_models.exe"
if (-not (Test-Path $Executable)) {
    Write-Error "Executable not found: $Executable"
    exit 1
}

Write-Host "Running pack_models with arguments:" -ForegroundColor Green
Write-Host ("  " + ($pack_models_args -join " ")) -ForegroundColor White

# 执行命令
& $Executable @pack_models_args

# 检查退出码
if ($LASTEXITCODE -ne 0) {
    Write-Error "pack_models failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}
else {
    Write-Host "Model packed successfully:" -ForegroundColor Green
    Write-Host "  Output: $outputFile"
}