# source build
# set subproject name
#project(kernel)  #vs 会以这个命名project命名来组织项目

set(LIB_NAME rs_core CACHE INTERNAL "core library name for rayshape") #目标跨文件夹缓存
# set kernelsource  
set(KERNEL_SOURCE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}) ##获取当前cmakelists路径
set(KERNEL_LINK_LIBRARY)  ##收集kernel库中需要链接所有内容
set(KERNEL_SOURCE)  # kernel source origin var 收集kennel所需要的源文件
set(KERNEL_WORK_FLODER kernel)
# include
include_directories(${KERNEL_SOURCE_ROOT_PATH}/include) #缩短源码包含路径长度

include(${KERNEL_SOURCE_ROOT_PATH}/../third_party/cmake/rapidjson.cmake)

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
        "${KERNEL_SOURCE_ROOT_PATH}/src/utils/*.cc")
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
    ## 其他推理引擎源码编译同理
    set(KERNEL_SOURCE ${KERNEL_SOURCE} ${INFERENCE_SOURCE})
endif()

# # TARGET
## add lib
add_library(${LIB_NAME} ${LIB_TYPE} ${KERNEL_SOURCE}) ## 

# #link lib
if(ENABLE_OPENVINO_INFERENCE)
    include(${ROOT_PATH}/third_party/cmake/openvino.cmake)## openvino的cmake宏函数
    set_openvino_lib()
    list(APPEND KERNEL_LINK_LIBRARY ${openvino_lib})
endif()

include(${ROOT_PATH}/third_party/cmake/rapidjson.cmake)

message(STATUS "lib name is:" ${LIB_NAME})

target_link_libraries(${LIB_NAME} ${KERNEL_LINK_LIBRARY})

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

