## module or sample 模块中公共链接的宏函数
## 可以考虑将kernel 编译成一个独立的库
macro(set_link_kernel)
    # Note: This should be handled per-target basis when needed
endmacro()

# function to link kernel for a target
function(target_link_kernel target_name)
    target_include_directories(${target_name}
        PRIVATE
            ${ROOT_PATH}/third
    )
endfunction()

macro(set_link_opencv)
    include(${ROOT_PATH}/third_party/cmake/opencv.cmake)
    set_opencv_lib(${ROOT_PATH}/third_party/) ## 宏函数的路径,set_opencv_lib中实现头文件包含和库添加
    list(APPEND RS_DEPLOY_LINK_LIBRARYS ${opecv_lib})
endmacro() ## module 模块连接RS_DEPLOY_LINK_LIBRARYS中的第三方库

macro(set_link_cereal)
    include(${ROOT_PATH}/third_party/cmake/cereal.cmake)
    set_cereal_lib() ## 配置cereal头文件包含
endmacro()
