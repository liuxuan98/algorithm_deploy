cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "RS_Deployment")
project(${PROJECT_NAME})
message("define name by self: ${PROJECT_NAME}")

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
option(ENABLE_STATIC_RUNTIME "Enable Static Runtime Library Linking" ON)
option(ENABLE_GLIBCXX_USE_CXX17_ABI "Enable Use CXX17 ABI" OFF)
option(ENABLE_GLIBCXX_USE_CXX14_ABI "Enable Use CXX14 ABI" ON)
option(ENABLE_GLIBCXX_USE_CXX11_ABI "Enable Use CXX11 ABI" OFF)
option(ENABLE_TIME_PROFILER "Enable Time Profiler " OFF)
option(ENABLE_RAPIDJSON "Enable RapidJson ThirdParty" ON)
option(ENABLE_CEREAL "Enable Cereal Serialization Library" ON)

# # base
option(ENABLE_BASE "Enable Base Compile" ON) ## 编译必须的macro

# # device
option(ENABLE_DEVICE "Enable Device" ON)
option(ENABLE_CPU_DEVICE "Enable Cpu Device" ON)
option(ENABLE_X86_DEVICE "Enable X86 Device" OFF)
option(ENABLE_OPENCL_DEVICE "Enable OpenCL Device" OFF)
option(ENABLE_CUDA_DEVICE "Enable Cuda Device" OFF)
if(NOT RS_CUDA_VERSION)
    set(RS_CUDA_VERSION 12.0)
endif()

# # inference
option(ENABLE_INFERENCE "Enable Inference" ON)
option(ENABLE_OPENVINO_INFERENCE  "Enable OpenVINO NetWork Inference" ON)  ## openvino的第三方库在kenel部分链接
option(ENABLE_MNN_INFERENCE  "Enable MNN NetWork Inference" OFF)
option(ENABLE_TENSORRT_INFERENCE  "Enable TensorRT NetWork Inference" OFF)
if(NOT RS_TENSORRT_VERSION)
    set(RS_TENSORRT_VERSION 10)
endif()
option(ENABLE_ONNXRUNTIME_INFERENCE  "Enable ONNXRuntime NetWork Inference" OFF)
if(NOT RS_ONNXRUNTIME_PROVIDER)
    set(RS_ONNXRUNTIME_PROVIDER cpu)  # cpu, cuda
endif()

# # model
option(ENABLE_MODEL "Enable Model" ON)
if(ENABLE_OPENVINO_INFERENCE)
    set(ENABLE_OPENVINO_MODEL ON)
endif()
if(ENABLE_MNN_INFERENCE)
    set(ENABLE_MNN_MODEL ON)
endif()
if(ENABLE_TENSORRT_INFERENCE)
    set(ENABLE_ONNX_MODEL ON)
    set(ENABLE_TENSORRT_MODEL ON)
endif()
if(ENABLE_ONNXRUNTIME_INFERENCE)
    set(ENABLE_ONNX_MODEL ON)
endif()

# # dag 有向无环图
option(ENABLE_DAG "Enable Directed Acyclic Graph" ON)
option(ENABLE_DAG_OPENCV "Enable " ON) # default ON ,

# # compole kernel submodule
option(ENABLE_KERNEL "Enable Complie kernel" ON)  ## 编译kernel 部分总开关,must on
option(ENABLE_SAMPLE "Enable Compilie Sample" ON)
option(ENABLE_TEST "Enable Complie Test" OFF)
option(ENABLE_TOOLS "Enable Complie Tools" ON)

# # set 3rd party install
option(ENABLE_3RD_OPENVINO "Enable 3rd Party" ON)
option(ENABLE_3RD_MNN "Enable 3rd Party" ON)
option(ENABLE_3RD_TENSORRT "Enable 3rd Party" ON)
option(ENABLE_3RD_ONNXRUNTIME "Enable 3rd Party" ON)
option(ENABLE_3RD_OPENCV "Enable 3rd Party" ON)

# # set root_path
message(${CMAKE_SOURCE_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})

set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "current ROOT_PATH dir is: ${ROOT_PATH}")

# folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# # define
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "set build type to release" FORCE)

if(${ENABLE_TIME_PROFILER} MATCHES "OFF")
else()
  add_definitions(-DENABLE_TIME_PROFILER)
endif()

if(ENABLE_CEREAL)
  add_definitions(-DENABLE_CEREAL)
endif()

# #compile 优化

message(STATUS ">>>>>>>>>>>>>")
message(STATUS "BUILD INFO:")
message(STATUS "\tSystem: ${CMAKE_SYSTEM_NAME}")
message(STATUS "\tProcessor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "\tBUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS ">>>>>>>>>>>>>")

#compile
include(${ROOT_PATH}/cmake/common/platform.cmake)

# #  dag rely thread pool
if(ENABLE_DAG)
    set(ENABLE_THREAD_POOL ON)
    set(ENABLE_CLASSIFICATION_PLUGIN ON)  #first plugin compile
    add_subdirectory(plugin)
    set(ENABLE_DAG_DEMO ON) # by the way add dag demo party.
endif()


# #compile kernel
if(ENABLE_KERNEL)
    add_subdirectory(kernel)
endif()

# if(ENABLE_MODULE)
#     add_subdirectory(module)
# endif()
message(STATUS "ENABLE_SAMPLE VALUE = ${ENABLE_SAMPLE}")

if(ENABLE_SAMPLE)
    add_subdirectory(sample)
endif()

if(ENABLE_TEST)
    add_subdirectory(test)
endif()

if(ENABLE_TOOLS)
    add_subdirectory(tools)
endif()
