# 安装库和头文件的宏函数
# 编译安装自有的库
macro(install_target ARGV0) #安装到build 目录下的bin/release
    message("installing target: ${ARGV0}")
    install(TARGETS ${ARGV0}
    RUNTIME DESTINATION bin/${CMAKE_BUILD_TYPE} #二进制文件路径
    LIBRARY DESTINATION lib/${CMAKE_BUILD_TYPE} #动态库的路径
    ARCHIVE DESTINATION lib/${CMAKE_BUILD_TYPE} #静态库
    #BUNDLE DESTINATION lib/${CMAKE_BUILD_TYPE} macOS 上的应用程序包
    PUBLIC_HEADER DESTINATION include)  ## 需要设置set_target_properties 为头文件属性才能安装
endmacro() ##相对路径是CMAKE_INSTALL_PREFIX的