## module or sample 模块中公共链接的宏函数
## 可以考虑将kernel 编译成一个独立的库
macro(set_link_kernel)
    include_directories(${ROOT_PATH}/third)
endmacro()

macro(set_link_opencv)
    include(${ROOT_PATH}/third_party/cmake/opencv.cmake)
    set_opencv_lib(${ROOT_PATH}/third_party/) ## 宏函数的路径,set_opencv_lib中实现头文件包含和库添加
    list(APPEND RS_DEPLOY_LINK_LIBRARYS ${opecv_lib})
endmacro() ## module 模块连接RS_DEPLOY_LINK_LIBRARYS中的第三方库