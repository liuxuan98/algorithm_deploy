# 获取脚本所在目录
$ScriptDir = Split-Path $MyInvocation.MyCommand.Path -Parent
. "$ScriptDir\lib_arg_parser.ps1" -ErrorAction Stop

# 定义必须参数和可选参数
$required_args = @("inference_type")
$optional_args = @("ort_provider", "trt_version", "cuda_version")

# 参数缩写映射
$arg_short_map = @{
    inference_type = "it"
    ort_provider   = "op"
    trt_version    = "tv"
    cuda_version   = "cv"
}

# 各参数的可选值（第一个为默认值）
$inference_type_vals = @("mnn", "openvino", "onnxruntime", "tensorrt")
$ort_provider_vals   = @("cpu", "cuda")
$trt_version_vals    = @("10", "8", "7")
$cuda_version_vals   = @("12", "11")

# 解析参数
ParseArgs $args

# 打印解析结果（调试用）
PrintArgs

# 计算项目根目录：脚本上级两级
$PROJECT_DIR = (Resolve-Path (Join-Path $ScriptDir "..\..")).Path
Write-Host "Project directory: $PROJECT_DIR"

# 三方库路径
$dll_paths = @(
    "$PROJECT_DIR\build\target\Windows\bin\Release",
    "$PROJECT_DIR\third_party\opencv\Windows\AMD64\bin\Release",
    "$PROJECT_DIR\third_party\RSLog\Windows\bin"
)

# 根据 inference_type 添加对应的第三方库路径
switch ($inference_type) {
    "mnn" {
        $dll_paths += "$PROJECT_DIR\third_party\mnn\Windows\AMD64\bin"
    }

    "openvino" {
        $dll_paths += "$PROJECT_DIR\third_party\openvino\Windows\AMD64\bin\intel64\Release"
        $dll_paths += "$PROJECT_DIR\third_party\openvino\Windows\AMD64\3rdparty\tbb\bin"
    }

    "onnxruntime" {
        # 设置ONNXRuntime路径没有作用
        $dll_paths += "$PROJECT_DIR\third_party\onnxruntime\$ort_provider\Windows\AMD64\lib\Release"
        Copy-Item -Path "$PROJECT_DIR\third_party\onnxruntime\$ort_provider\Windows\AMD64\lib\Release\*.dll" -Destination "$PROJECT_DIR\build\target\Windows\bin\Release"
        if ($ort_provider -eq "cuda") {
            $dll_paths += "$PROJECT_DIR\third_party\cudnn\8.9-12.x\Windows\AMD64\bin\Release"
            $dll_paths += "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v${cuda_version}.0\bin"
        }
    }

    "tensorrt" {
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

    default {
        Write-Error "Unsupported inference_type: $inference_type"
        exit 1
    }
}


# 设置环境变量 dll_paths（仅在 WSL\Windows 环境中生效）
$env:PATH = ($dll_paths -join ';') + ";" + $env:PATH
Write-Host "PATH set to:" -ForegroundColor Cyan
Write-Host $env:PATH

# 创建输出目录
$OutputDir = "$PROJECT_DIR\output\$inference_type"
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
Write-Host "Output directory: $OutputDir"

# 构建模型路径
$ModelName = "checkpoint-best-${inference_type}.rsm"
$ModelPath = "$PROJECT_DIR\model\breast_thyroid\rsm\windows\$ModelName"

# 检查模型是否存在
if (-not (Test-Path $ModelPath)) {
    Write-Error "Model file not found: $ModelPath"
    exit 1
}

# 构建可执行文件路径
$Executable = "$PROJECT_DIR\build\target\Windows\bin\Release\image_classifier_sample.exe"

# 检查可执行文件是否存在
if (-not (Test-Path $Executable)) {
    Write-Error "Executable not found: $Executable"
    exit 1
}

# 执行命令
$CommandArgs = @("images", $ModelPath, $inference_type)
Write-Host "Running command: $Executable $CommandArgs" -ForegroundColor Green

& $Executable @CommandArgs

# 传递退出码
if ($LASTEXITCODE -ne 0) {
    Write-Error "Execution failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}
else {
    Write-Host "Sample executed successfully." -ForegroundColor Green
}