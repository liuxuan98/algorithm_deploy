# lib_arg_parser.ps1 - 通用参数解析库

# 使用示例：
# test.ps1 - 使用通用参数解析库进行输入参数解析
#   $ScriptDir = Split-Path $MyInvocation.MyCommand.Path -Parent
#   . "$ScriptDir\lib_arg_parser.ps1"
#
#   # 定义参数
#   $required_args = @("test_name")
#   $optional_args = @("verbose")
#
#   # 参数缩写映射
#   $arg_short_map = @{
#       test_name = "n"
#       verbose   = "v"
#   }
#
#   # 可选值（第一个为默认值）
#   $test_name_vals = @("flycv", "ppl.cv")
#   $verbose_vals   = @("off", "on")
#
#   # 解析参数
#   ParseArgs @args
#
#   # 打印结果
#   PrintArgs


# 工具函数：判断值是否在数组中（忽略大小写）
function Contains {
    param(
        [string]$Value,
        [string[]]$Array
    )
    if ($null -eq $Array -or $Array.Count -eq 0) {
        return $false
    }
    $lowerValue = $Value.ToLower()
    foreach ($item in $Array) {
        if ($item.ToLower() -eq $lowerValue) {
            return $true
        }
    }
    return $false
}

# 生成帮助信息
function Usage {
    param(
        [int]$ExitCode = 1
    )

    $scriptName = $MyInvocation.ScriptName
    if ([string]::IsNullOrEmpty($scriptName)) {
        $scriptName = "script.ps1"
    } else {
        $scriptName = Split-Path $scriptName -Leaf
    }

    Write-Host "Usage: $scriptName" -ForegroundColor Green

    Write-Host "  --help, -h  # display help information"

    # 输出必须参数
    foreach ($arg in $required_args) {
        $short = ""
        if ($arg_short_map.ContainsKey($arg) -and $arg_short_map[$arg]) {
            $short = ", -$($arg_short_map[$arg])"
        }
        $valsVar = Get-Variable -Name "${arg}_vals" -ValueOnly -ErrorAction SilentlyContinue
        if ($valsVar -and $valsVar.Count -gt 0) {
            $allowed = $valsVar -join " "
            Write-Host "  --$arg$short <$arg>  # allowed values: $allowed"
        } else {
            Write-Host "  --$arg$short <$arg>"
        }
    }

    # 输出可选参数（用方括号表示可选）
    foreach ($arg in $optional_args) {
        $short = ""
        if ($arg_short_map.ContainsKey($arg) -and $arg_short_map[$arg]) {
            $short = ", -$($arg_short_map[$arg])"
        }
        $valsVar = Get-Variable -Name "${arg}_vals" -ValueOnly -ErrorAction SilentlyContinue
        if ($valsVar -and $valsVar.Count -gt 0) {
            $allowed = $valsVar -join " "
            Write-Host "  [--$arg$short <$arg>]  # allowed values: $allowed"
        } else {
            Write-Host "  [--$arg$short <$arg>]"
        }
    }

    exit $ExitCode
}

# 构建短参数到长参数的映射
function BuildShortMap {
    $script:short_to_long = @{}
    foreach ($arg in $required_args + $optional_args) {
        if ($arg_short_map.ContainsKey($arg)) {
            $short = $arg_short_map[$arg]
            if ($short) {
                $script:short_to_long["-$short"] = "--$arg"
            }
        }
    }
}

# 检查参数合法性
function CheckArgs {
    foreach ($arg in $required_args) {
        $val = Get-Variable -Name $arg -ValueOnly -ErrorAction SilentlyContinue
        if ([string]::IsNullOrEmpty($val)) {
            Write-Error "Missing required argument '--$arg'"
            Usage 1
        }

        $valsVar = Get-Variable -Name "${arg}_vals" -ValueOnly -ErrorAction SilentlyContinue
        if ($valsVar -and $valsVar.Count -gt 0) {
            if (-not (Contains -Value $val -Array $valsVar)) {
                Write-Error "Invalid value for ${arg}: $val"
                Write-Host "Allowed values: $($valsVar -join ' ')"
                Usage 1
            }
        }
    }

    foreach ($arg in $optional_args) {
        $val = Get-Variable -Name $arg -ValueOnly -ErrorAction SilentlyContinue
        if (-not [string]::IsNullOrEmpty($val)) {
            $valsVar = Get-Variable -Name "${arg}_vals" -ValueOnly -ErrorAction SilentlyContinue
            if ($valsVar -and $valsVar.Count -gt 0) {
                if (-not (Contains -Value $val -Array $valsVar)) {
                    Write-Error "Invalid value for ${arg}: $val"
                    Write-Host "Allowed values: $($valsVar -join ' ')"
                    Usage 1
                }
            }
        }
    }
}

# 为可选参数设置默认值
function ApplyDefaults {
    foreach ($arg in $optional_args) {
        $val = Get-Variable -Name $arg -ValueOnly -ErrorAction SilentlyContinue
        if ([string]::IsNullOrEmpty($val)) {
            $valsVar = Get-Variable -Name "${arg}_vals" -ValueOnly -ErrorAction SilentlyContinue
            if ($valsVar -and $valsVar.Count -gt 0) {
                Set-Variable -Name $arg -Value $valsVar[0] -Scope Script
            }
        }
    }
}

# 解析命令行参数
function ParseArgs {
    param(
        [string[]]$Arguments
    )

    $script:provided_args = @()

    BuildShortMap

    $i = 0
    while ($i -lt $Arguments.Length) {
        $arg = $Arguments[$i]
        if ($arg -eq "--help" -or $arg -eq "-h") {
            Usage 0
        }

        # 检查是否为短参数
        $expandedArg = $arg
        if ($script:short_to_long.ContainsKey($arg)) {
            $expandedArg = $script:short_to_long[$arg]
        }

        $matched = $false
        foreach ($paramName in $required_args + $optional_args) {
            if ($expandedArg -eq "--$paramName") {
                $i++
                if ($i -ge $Arguments.Length) {
                    $displayArg = if ($script:short_to_long.ContainsKey($arg)) { $arg } else { "--$paramName" }
                    Write-Error "Missing value after $displayArg"
                    Usage 1
                }
                $value = $Arguments[$i]
                Set-Variable -Name $paramName -Value $value -Scope Script
                $script:provided_args += $paramName
                $matched = $true
                break
            }
        }

        if (-not $matched) {
            Write-Error "Unknown option: $arg"
            Usage 1
        }

        $i++
    }
    CheckArgs
    ApplyDefaults
}

# 打印解析结果
function PrintArgs {
    Write-Host "--- args:" -ForegroundColor Cyan

    # 输出命令行传入的参数
    foreach ($arg in $script:provided_args) {
        $val = Get-Variable -Name $arg -ValueOnly -Scope Script
        Write-Host "  ${arg}: $val"
    }

    # 输出未传入但有默认值的可选参数
    foreach ($arg in $optional_args) {
        if ($script:provided_args -notcontains $arg) {
            $val = Get-Variable -Name $arg -ValueOnly -Scope Script
            Write-Host "  ${arg}: $val (default)"
        }
    }

    Write-Host "---" -ForegroundColor Cyan
}