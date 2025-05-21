cmake_minimum_required(VERSION 3.10)

set(MY_PROJECT_NAME "RS_Deployment")
project(${MY_PROJECT_NAME})
message("define name by self: ${MY_PROJECT_NAME}")

# project version
set(RS_MAJOR_VERSION 0)
set(RS_MINOR_VERSION 3)
set(RS_PATCH_VERSION 0)
set(RS_BUILD_VERSION 0)
set(RS_VERSION "${RS_MAJOR_VERSION}.${RS_MINOR_VERSION}.${RS_PATCH_VERSION}.${RS_BUILD_VERSION}")


# base macro
option(ENABLE_BUILD_SHARED "Build Shared Library" ON)
option(ENABLE_SYMBOL_HIDE "Enable Symbol Hide" OFF)
option(ENABLE_COVERAGE "Enable Coverage" OFF) ##注意变量的缩进可能会导致 导致cmake的警告
option(ENABLE_GLIBCXX_USE_CXX17_ABI "Enable Use CXX17 ABI" ON)
option(ENABLE_GLIBCXX_USE_CXX14_ABI "Enable Use CXX14 ABI" OFF)
option(ENABLE_GLIBCXX_USE_CXX11_ABI "Enable Use CXX11 ABI" OFF)
option(ENABLE_TIME_PROFILER "Enable Time Profiler " OFF)

# # base
option(ENABLE_BASE "Enable Base Compile" ON) ## 编译必须的macro 

# # device
option(ENABLE_DEVICE "Enable Device" ON)
option(ENABLE_CPU_DEVICE "Enable Cpu Device" ON)
option(ENABLE_X86_DEVICE "Enable X86 Device" OFF)
option(ENABLE_OPENCL_DEVICE "Enable OpenCL Device" OFF)
option(ENABLE_CUDA_DEVICE "Enable Cuda Device" OFF)

# # inference
option(ENABLE_INFERENCE "Enable Inference" ON)
option(ENABLE_OPENVINO_INFERENCE  "Enable OpenVINO NetWork Inference" ON)  ## openvino的第三方库在kenel部分链接

# # compole kernel submodule
option(ENABLE_KERNEL "Enable Complie kernel" ON)  ## 编译kernel 部分总开关,must on

# # set root_path
message(${CMAKE_SOURCE_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})

set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "current ROOT_PATH dir is: ${ROOT_PATH}")


# # define
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "set build type to release" FORCE)

if(${ENABLE_TIME_PROFILER} MATCHES "OFF")
else()
  add_definitions(-DENABLE_TIME_PROFILER)
endif()

# #compile 优化

message(STATUS ">>>>>>>>>>>>>")
message(STATUS "TNN BUILD INFO:")
message(STATUS "\tSystem: ${CMAKE_SYSTEM_NAME}")
message(STATUS "\tProcessor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "\tBUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS ">>>>>>>>>>>>>")

# install path
set(PROJECT_INSTALL_PATH ${ROOT_PATH}/build/install)
set(CMAKE_INSTALL_PREFIX ${PROJECT_INSTALL_PATH})

#compile
include(${ROOT_PATH}/cmake/common/platform.cmake)


# #compile kernel
if(ENABLE_KERNEL)
    add_subdirectory(kernel)
endif()

# if(ENABLE_TEST)
#     add_subdirectory(test)
# endif()

# if(ENABLE_MODULE)
#     add_subdirectory(module)
# endif()

# if(ENABLE_SAMPLE)
#     add_subdirectory(sample)
# endif()






