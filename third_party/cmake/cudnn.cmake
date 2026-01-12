# 前缀 lib  后缀 .so .a .dll、.dylib 依据平台进行判断链接 和 动态库还是静态库进行连接 ，要求统一动态库

# 定义宏：自动配置 TensorRT 头文件与库
macro(set_cudnn_lib version)

    set(cudnn_lib) #全局存放变量

    set(libs "cudnn")

    #根据平台拼接库 cudnn_lib_path 会有不同变化
    set(cudnn_lib_path ${ROOT_PATH}/third_party/cudnn/${version}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/Release)
    message("cuDNN Version: " ${version})
    
    foreach(lib ${libs})
        set(lib_name ${LIB_PREFIX}${lib}${LIB_SUFFIX})
        set(full_lib_name ${cudnn_lib_path}/${lib_name})
        list(APPEND cudnn_lib ${full_lib_name})
    endforeach()

    unset(lib_name)
    unset(full_lib_name)
    unset(cudnn_lib_path)
    unset(cudnn_include_dir)
    unset(libs)

endmacro()

# Function to configure cuDNN includes for a target
function(target_link_cudnn target_name version)
    set_cudnn_lib(${version})
    target_link_libraries(${target_name} PRIVATE ${cudnn_lib})
    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${ROOT_PATH}/third_party/cudnn/${version}/include
    )

    set(CUDNN_LIB_PATH
        ${ROOT_PATH}/third_party/cudnn/${version}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/Release)
    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH "${CUDNN_LIB_PATH}"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    target_link_directories(${target_name} PRIVATE ${CUDNN_LIB_PATH})
endfunction()