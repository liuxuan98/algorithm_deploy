# 前缀 lib  后缀 .so .a .dll、.dylib 依据平台进行判断链接 和 动态库还是静态库进行连接 ，要求统一动态库

# 定义宏：自动配置 TensorRT 头文件与库
macro(set_tensorrt_lib version)

    set(tensorrt_lib) #全局存放变量

    set(libs "nvinfer;nvonnxparser;nvinfer_plugin")

    #根据平台拼接库 tensorrt_lib_path 会有不同变化
    set(tensorrt_lib_path ${ROOT_PATH}/third_party/tensorrt/${version}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/Release)
    message("TensorRT Version: " ${version})
    
    foreach(lib ${libs})
        if(SYSTEM.Windows)
            if("${version}" STREQUAL "10")
                set(lib_name ${LIB_PREFIX}${lib}_10${LIB_SUFFIX})
            else()
                set(lib_name ${LIB_PREFIX}${lib}${LIB_SUFFIX})
            endif()
        else()
            set(lib_name ${LIB_PREFIX}${lib}${LIB_SUFFIX})
        endif()
        set(full_lib_name ${tensorrt_lib_path}/${lib_name})
        list(APPEND tensorrt_lib ${full_lib_name})
    endforeach()

    unset(lib_name)
    unset(full_lib_name)
    unset(tensorrt_lib_path)
    unset(libs)

endmacro()

# Function to configure TensorRT includes for a target
function(target_link_tensorrt target_name version)
    set_tensorrt_lib(${version})
    target_link_libraries(${target_name} PRIVATE ${tensorrt_lib})
    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${ROOT_PATH}/third_party/tensorrt/${version}/include
    )

    set(TENSORRT_LIB_PATH
        ${ROOT_PATH}/third_party/tensorrt/${version}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/Release)
    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH "${TENSORRT_LIB_PATH}"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    target_link_directories(${target_name} PRIVATE ${TENSORRT_LIB_PATH})
endfunction()