@echo off

REM 获取当前脚本路径
set SCRIPT_DIR=%~dp0%
set ROOT_DIR=%SCRIPT_DIR%..\..

REM 检查是否传入了构建类型参数，否则默认为 Release
if "%1"=="" (
    set BUILD_TYPE=Release
) else (
    set BUILD_TYPE=%1
)

set ARCH=%2
if "%ARCH%"=="" (
    REM 自动检测当前系统架构
    if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
        set ARCH=x64
    ) else if "%PROCESSOR_ARCHITEW6432%"=="AMD64" (
        set ARCH=x64
    ) else (
        set ARCH=x86
    )
)

REM 定位到上两级目录
cd /d "%ROOT_DIR%"
echo Current directory: %cd%

REM 创建并进入 build 目录
if exist build (
    echo Cleaning existing build directory...
    rmdir /s /q build
)

mkdir build
cd build

REM 配置 CMake 使用 MSVC 编译器
cmake -G "Visual Studio 16 2019" -A %ARCH% ^
    -DCMAKE_SYSTEM_NAME=Windows ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_SYSTEM_PROCESSOR=%PROCESSOR_ARCHITECTURE% ^
    -DENABLE_BUILD_SHARED=ON ^
    -DENABLE_SYMBOL_HIDE=OFF ^
    -DENABLE_COVERAGE=OFF ^
    -DENABLE_GLIBCXX_USE_CXX14_ABI=ON ^
    -DENABLE_TIME_PROFILER=OFF ^
    -DENABLE_RAPIDJSON=ON ^
    -DENABLE_OPENVINO_INFERENCE=ON ^
    -DENABLE_TEST=ON ..

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

REM 构建项目 (可选: --target 指定具体目标)
echo Building project with configuration: %BUILD_TYPE%...
cmake --build . --config %BUILD_TYPE%

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed. Please check the output for details.
    exit /b 1
)

REM echo Build completed.
echo Build completed successfully.