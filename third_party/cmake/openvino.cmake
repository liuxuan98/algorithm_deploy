# 前缀 lib  后缀 .so .a .dll、.dylib 依据平台进行判断链接 和 动态库还是静态库进行连接 ，要求统一动态库

# 定义宏：自动配置 OpenVINO 头文件与库
macro(set_openvino_lib)

    set(openvino_lib) #全局存放变量

    set(libs "openvino")
    #包含头文件
    set(openvino_include_dir ${ROOT_PATH}/third_party/openvino/include)
    include_directories(${openvino_include_dir})

    #根据平台拼接库 openvino_lib_path 会有不同变化
    set(openvino_lib_path ${ROOT_PATH}/third_party/openvino/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/${CMAKE_BUILD_TYPE})
    
    foreach(lib ${libs})
        set(lib_name ${LIB_PREFIX}${lib}${LIB_SUFFIX})
        set(full_lib_name ${openvino_lib_path}/${lib_name})
        list(APPEND openvino_lib ${full_lib_name})
    endforeach()    

    unset(lib_name)
    unset(full_lib_name)
    unset(openvino_lib_path)
    unset(openvino_include_dir)
    unset(libs)

endmacro()