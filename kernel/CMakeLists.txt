# source build
# set subproject name
#project(kernel)  #vs 会以这个命名project命名来组织项目

set(LIB_NAME rs_core CACHE INTERNAL "core library name for rayshape") #目标跨文件夹缓存
# set kernelsource
set(KERNEL_SOURCE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}) ##获取当前cmakelists路径
set(KERNEL_LINK_LIBRARY)  ##收集kernel库中需要链接所有内容
set(KERNEL_SOURCE)  # kernel source origin var 收集kennel所需要的源文件
set(KERNEL_WORK_FLODER kernel)

include(${KERNEL_SOURCE_ROOT_PATH}/../third_party/cmake/rapidjson.cmake)
include(${KERNEL_SOURCE_ROOT_PATH}/../third_party/cmake/zlib.cmake)
include(${KERNEL_SOURCE_ROOT_PATH}/../third_party/cmake/rslog.cmake)

# include opencv if enabled and add definitions.
if(ENABLE_DAG_OPENCV)
    add_definitions(-DENABLE_3RD_OPENCV) #dag部分启用opencv
    #target_compile_definitions(${LIB_NAME} PRIVATE ENABLE_3RD_OPENCV)
    include(${KERNEL_SOURCE_ROOT_PATH}/../third_party/cmake/opencv.cmake) # default link openCV. 
endif()

# Include cereal if enabled
if(ENABLE_CEREAL)
    include(${KERNEL_SOURCE_ROOT_PATH}/../third_party/cmake/cereal.cmake)
endif()

# Include CryptoPP for encryption support
include(${KERNEL_SOURCE_ROOT_PATH}/../third_party/cmake/cryptopp.cmake)

#set()
#根据最外层的cmake编译选项来添加编译源文件
# # attention GLOB or GLOB_RECURSE
# base source(必须要编译进来的)
if(ENABLE_BASE)
    file(GLOB BASE_SOURCE
        "${KERNEL_SOURCE_ROOT_PATH}/include/base/*.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/base/*.cc"
    )
    set(KERNEL_SOURCE ${KERNEL_SOURCE} ${BASE_SOURCE}) #将KERNEL_SOURCE and BASE_SOURCE merge one KERNEL_SOURCE.

    # utils
    file(GLOB UTILS_SOURCE
        "${KERNEL_SOURCE_ROOT_PATH}/include/utils/*.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/utils/*.cc"
        "${KERNEL_SOURCE_ROOT_PATH}/include/utils/codec/*.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/utils/codec/*.cc"
    )
    set(KERNEL_SOURCE ${KERNEL_SOURCE} ${UTILS_SOURCE})
    #memory_management
    file(GLOB MEMORY_MANAGEMENT_SOURCE
        "${KERNEL_SOURCE_ROOT_PATH}/include/memory_manager/*.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/memory_manager/*.cc"
        "${KERNEL_SOURCE_ROOT_PATH}/include/memory_manager/kernel_buffer.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/memory_manager/kernel_buffer.cc")
    set(KERNEL_SOURCE ${KERNEL_SOURCE} ${MEMORY_MANAGEMENT_SOURCE})
endif()


# 使能设备
if(ENABLE_DEVICE)
    file(GLOB DEVICE_SOURCE
        "${KERNEL_SOURCE_ROOT_PATH}/include/device/*.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/device/*.cc")
    ## 暂时只有cpu设备

    file(GLOB_RECURSE DEVICE_CPU_SOURCE
            "${KERNEL_SOURCE_ROOT_PATH}/include/device/cpu/*.h"
            "${KERNEL_SOURCE_ROOT_PATH}/src/device/cpu/*.cc")
    set(DEVICE_SOURCE ${DEVICE_SOURCE} ${DEVICE_CPU_SOURCE})

    if(ENABLE_CUDA_DEVICE)
        file(GLOB_RECURSE DEVICE_CUDA_SOURCE
                "${KERNEL_SOURCE_ROOT_PATH}/include/device/cuda/*.h"
                "${KERNEL_SOURCE_ROOT_PATH}/src/device/cuda/*.cc")
        set(DEVICE_SOURCE ${DEVICE_SOURCE} ${DEVICE_CUDA_SOURCE})
    endif()

    set(KERNEL_SOURCE ${KERNEL_SOURCE} ${DEVICE_SOURCE})
endif()

# 使能推理引擎
if(ENABLE_INFERENCE)
    file(GLOB INFERENCE_SOURCE
        "${KERNEL_SOURCE_ROOT_PATH}/include/inference/*.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/inference/*.cc")
    if(ENABLE_OPENVINO_INFERENCE)
        file(GLOB_RECURSE INFERENCE_OPENVINO_SOURCE
            "${KERNEL_SOURCE_ROOT_PATH}/include/inference/openvino/*.h"
            "${KERNEL_SOURCE_ROOT_PATH}/src/inference/openvino/*.cc")
        set(INFERENCE_SOURCE ${INFERENCE_SOURCE} ${INFERENCE_OPENVINO_SOURCE})
    endif()

    if(ENABLE_MNN_INFERENCE)
        file(GLOB_RECURSE INFERENCE_MNN_SOURCE
            "${KERNEL_SOURCE_ROOT_PATH}/include/inference/mnn/*.h"
            "${KERNEL_SOURCE_ROOT_PATH}/src/inference/mnn/*.cc")
        set(INFERENCE_SOURCE ${INFERENCE_SOURCE} ${INFERENCE_MNN_SOURCE})
    endif()
    if(ENABLE_TENSORRT_INFERENCE)
        file(GLOB_RECURSE INFERENCE_TENSORRT_SOURCE
            "${KERNEL_SOURCE_ROOT_PATH}/include/inference/tensorrt/*.h"
            "${KERNEL_SOURCE_ROOT_PATH}/src/inference/tensorrt/*.cc")
        set(INFERENCE_SOURCE ${INFERENCE_SOURCE} ${INFERENCE_TENSORRT_SOURCE})
    endif()
    if(ENABLE_ONNXRUNTIME_INFERENCE)
        file(GLOB_RECURSE INFERENCE_ONNXRUNTIME_SOURCE
            "${KERNEL_SOURCE_ROOT_PATH}/include/inference/onnxruntime/*.h"
            "${KERNEL_SOURCE_ROOT_PATH}/src/inference/onnxruntime/*.cc")
        set(INFERENCE_SOURCE ${INFERENCE_SOURCE} ${INFERENCE_ONNXRUNTIME_SOURCE})

        if("${RS_ONNXRUNTIME_PROVIDER}" STREQUAL "cuda")
            add_definitions(-DRS_ONNXRUNTIME_PROVIDER_CUDA)
        endif()
    endif()
    ## 其他推理引擎源码编译同理
    set(KERNEL_SOURCE ${KERNEL_SOURCE} ${INFERENCE_SOURCE})
endif()

# 使能模型
if(ENABLE_MODEL)
    file(GLOB MODEL_SOURCE
        "${KERNEL_SOURCE_ROOT_PATH}/include/model/*.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/model/*.cc")

    # mnn model
    if(ENABLE_MNN_MODEL)
        add_definitions(-DENABLE_MNN_MODEL)
        file(GLOB_RECURSE MODEL_MNN_SOURCE
            "${KERNEL_SOURCE_ROOT_PATH}/include/model/mnn/*.h"
            "${KERNEL_SOURCE_ROOT_PATH}/src/model/mnn/*.cc")
        set(MODEL_SOURCE ${MODEL_SOURCE} ${MODEL_MNN_SOURCE})
    endif()

    # openvino model
    if(ENABLE_OPENVINO_MODEL)
        add_definitions(-DENABLE_OPENVINO_MODEL)
        file(GLOB_RECURSE MODEL_OPENVINO_SOURCE
            "${KERNEL_SOURCE_ROOT_PATH}/include/model/openvino/*.h"
            "${KERNEL_SOURCE_ROOT_PATH}/src/model/openvino/*.cc")
        set(MODEL_SOURCE ${MODEL_SOURCE} ${MODEL_OPENVINO_SOURCE})
    endif()

    # onnx model
    if(ENABLE_ONNX_MODEL)
        add_definitions(-DENABLE_ONNX_MODEL)
        file(GLOB_RECURSE MODEL_ONNX_SOURCE
            "${KERNEL_SOURCE_ROOT_PATH}/include/model/onnx/*.h"
            "${KERNEL_SOURCE_ROOT_PATH}/src/model/onnx/*.cc")
        set(MODEL_SOURCE ${MODEL_SOURCE} ${MODEL_ONNX_SOURCE})
    endif()

    set(KERNEL_SOURCE ${KERNEL_SOURCE} ${MODEL_SOURCE})
endif()

## 编译有向无环图源码.
if(ENABLE_DAG)
    file(GLOB_RECURSE DAG_SOURCE
    "${KERNEL_SOURCE_ROOT_PATH}/include/dag/*.h"
    "${KERNEL_SOURCE_ROOT_PATH}/src/dag/*.cc" 
    "${KERNEL_SOURCE_ROOT_PATH}/src/dag/engine/*.cc"
    "${KERNEL_SOURCE_ROOT_PATH}/src/dag/engine/parallel_pipeline_engine.cc"
    "${KERNEL_SOURCE_ROOT_PATH}/src/dag/edge/pipeline_edge.cc")
    if(ENABLE_THREAD_POOL)
        file(GLOB_RECURSE THREAD_POOL_SOURCE
        "${KERNEL_SOURCE_ROOT_PATH}/include/thread_pool/*.h"
        "${KERNEL_SOURCE_ROOT_PATH}/src/thread_pool/*.cc")
        set(DAG_SOURCE ${DAG_SOURCE} ${THREAD_POOL_SOURCE})
    endif()
    set(KERNEL_SOURCE ${KERNEL_SOURCE} ${DAG_SOURCE})
endif()

# # TARGET
if(ENABLE_CUDA_DEVICE)
    include(${ROOT_PATH}/third_party/cmake/cuda.cmake)## cuda的cmake宏函数
    set_cuda_lib(${RS_CUDA_VERSION})
    list(APPEND KERNEL_LINK_LIBRARY ${cuda_lib})
endif()

## add lib
add_library(${LIB_NAME} ${LIB_TYPE} ${KERNEL_SOURCE}) ##

target_include_directories(${LIB_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${KERNEL_SOURCE_ROOT_PATH}/include>
        $<INSTALL_INTERFACE:include>
)

# Configure third-party libraries
# Note: The include files are already loaded above, so we can use the modern functions
target_link_rapidjson(${LIB_NAME})
target_link_zlib(${LIB_NAME})
target_link_rslog(${LIB_NAME})

# Include opencv if enabled
if(ENABLE_DAG_OPENCV)
    target_link_opencv(${LIB_NAME})
endif()

# Include cereal if enabled
if(ENABLE_CEREAL)
    target_link_cereal(${LIB_NAME})
endif()

# Configure CryptoPP - include the cmake file first
include(${KERNEL_SOURCE_ROOT_PATH}/../third_party/cmake/cryptopp.cmake)
check_cryptopp_available(CRYPTOPP_AVAILABLE)
if(CRYPTOPP_AVAILABLE)
    target_link_cryptopp(${LIB_NAME})
    add_definitions(-DENABLE_CRYPTOPP)
    message(STATUS "CryptoPP encryption support enabled")
else()
    message(WARNING "CryptoPP not available, encryption features will be disabled")
endif()

# Configure inference engines
if(ENABLE_OPENVINO_INFERENCE)
    include(${ROOT_PATH}/third_party/cmake/openvino.cmake)## openvino的cmake宏函数
    target_link_openvino(${LIB_NAME})
endif()

if(ENABLE_MNN_INFERENCE)
    include(${ROOT_PATH}/third_party/cmake/mnn.cmake)## mnn的cmake宏函数
    target_link_mnn(${LIB_NAME})
endif()

if(ENABLE_ONNXRUNTIME_INFERENCE)
    include(${ROOT_PATH}/third_party/cmake/onnxruntime.cmake)## onnxruntime的cmake宏函数
    target_link_onnxruntime(${LIB_NAME})
endif()
if(ENABLE_TENSORRT_INFERENCE)
    include(${ROOT_PATH}/third_party/cmake/tensorrt.cmake)## tensorrt的cmake宏函数
    if("${RS_TENSORRT_VERSION}" STREQUAL "8")
        if("${RS_CUDA_VERSION}" MATCHES "^11\\.[0-9]+$")
            target_link_tensorrt(${LIB_NAME} ${RS_TENSORRT_VERSION}-11.x)
        elseif("${RS_CUDA_VERSION}" MATCHES "^12\\.[0-9]+$")
            target_link_tensorrt(${LIB_NAME} ${RS_TENSORRT_VERSION}-12.x)
        endif()
    else()
        target_link_tensorrt(${LIB_NAME} ${RS_TENSORRT_VERSION})
    endif()

    if("${RS_TENSORRT_VERSION}" STREQUAL "8" OR "${RS_TENSORRT_VERSION}" STREQUAL "7")
        include(${ROOT_PATH}/third_party/cmake/cudnn.cmake)
        if("${RS_CUDA_VERSION}" MATCHES "^11\\.[0-9]+$")
            target_link_cudnn(${LIB_NAME} 8.1-11.x)
        elseif("${RS_CUDA_VERSION}" MATCHES "^12\\.[0-9]+$")
            target_link_cudnn(${LIB_NAME} 8.9-12.x)
        endif()
    endif()
endif()

# Legacy linking for backward compatibility (some libraries may still need this)
if(ZLIB_FOUND AND NOT TARGET ZLIB::ZLIB)
    list(APPEND KERNEL_LINK_LIBRARY ${ZLIB_LIBRARIES})
endif()

message(STATUS "lib name is:" ${LIB_NAME})

target_link_libraries(${LIB_NAME} PRIVATE ${KERNEL_LINK_LIBRARY})

#设置项目的目标分组
set_property(TARGET ${LIB_NAME} PROPERTY FOLDER ${KERNEL_WORK_FLODER})

## install
include(${ROOT_PATH}/cmake/common/install_macro.cmake)
install_target(${LIB_NAME})

#unset(LIB_NAME) #防止目标丢失
unset(KERNEL_SOURCE)
unset(KERNEL_SOURCE_ROOT_PATH)
unset(KERNEL_LINK_LIBRARY)
unset(KERNEL_WORK_FLODER)
