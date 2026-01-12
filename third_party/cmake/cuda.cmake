# 定义宏：自动配置 CUDA 头文件与库
macro(set_cuda_lib version)

    option(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
    if(SYSTEM.Linux)
        set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda-${version})
    elseif(SYSTEM.Windows)
        set(CUDA_TOOLKIT_ROOT_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v${version}")
    endif()
    if(POLICY CMP0146)
        cmake_policy(SET CMP0146 OLD)
    endif()
    find_package(CUDA REQUIRED)
    message("CUDA Version: " ${CUDA_VERSION})
    include_directories(${CUDA_INCLUDE_DIRS})

    set(cuda_lib ${CUDA_LIBRARIES} ) #全局存放变量

endmacro()