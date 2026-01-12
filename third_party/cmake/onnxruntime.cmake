# 前缀 lib  后缀 .so .a .dll、.dylib 依据平台进行判断链接 和 动态库还是静态库进行连接 ，要求统一动态库

# 定义宏：自动配置 ONNXRuntime 头文件与库
macro(set_onnxruntime_lib)

    set(onnxruntime_lib) #全局存放变量

    set(libs "onnxruntime")
    #包含头文件

    #根据平台拼接库 onnxruntime_lib_path 会有不同变化
    set(onnxruntime_lib_path ${ROOT_PATH}/third_party/onnxruntime/${RS_ONNXRUNTIME_PROVIDER}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/${CMAKE_BUILD_TYPE})
    
    foreach(lib ${libs})
        set(lib_name ${LIB_PREFIX}${lib}${LIB_SUFFIX})
        set(full_lib_name ${onnxruntime_lib_path}/${lib_name})
        list(APPEND onnxruntime_lib ${full_lib_name})
    endforeach()

    unset(lib_name)
    unset(full_lib_name)
    unset(onnxruntime_lib_path)
    unset(libs)

endmacro()

# Function to configure ONNXRuntime includes for a target
function(target_link_onnxruntime target_name)
    set_onnxruntime_lib()
    target_link_libraries(${target_name} PRIVATE ${onnxruntime_lib})
    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${ROOT_PATH}/third_party/onnxruntime/${RS_ONNXRUNTIME_PROVIDER}/include
    )

    set(ONNXRUNTIME_LIB_PATH
        ${ROOT_PATH}/third_party/onnxruntime/${RS_ONNXRUNTIME_PROVIDER}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/${CMAKE_BUILD_TYPE})
    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH "${ONNXRUNTIME_LIB_PATH}"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    target_link_directories(${target_name} PRIVATE ${ONNXRUNTIME_LIB_PATH})
endfunction()