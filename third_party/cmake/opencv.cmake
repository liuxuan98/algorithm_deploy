# 定义宏：自动配置 OpenCV 头文件与库
macro(set_opencv_lib ARG0)
    
    message(STATUS "set_opencv_lib: ${ARG0}")
    
    if("${ARG0}" STREQUAL "")
        set(libs "core452" "imgproc452" "highgui452") ## 非world的cv库，核心链接的三个库
    elseif("${ARG0}" STREQUAL "world")
        set(libs "opencv_world452")
    endif()

    set(opencv_lib)
    #包含头文件
    set(opencv_include_dir ${ROOT_PATH}/third_party/opencv/include)
    include_directories(${opencv_include_dir})

    #根据平台拼接库 opencv_lib_path 会有不同变化
    set(opencv_lib_path ${ROOT_PATH}/third_party/opencv/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/${CMAKE_BUILD_TYPE})
    
    foreach(lib ${libs})
        set(lib_name ${LIB_PREFIX}${lib}${LIB_SUFFIX})
        set(full_lib_name ${opencv_lib_path}/${lib_name})
        list(APPEND opencv_lib ${full_lib_name})
    endforeach()

    unset(libs)
    unset(opencv_include_dir)
    unset(opencv_lib_path)
    unset(lib_name)
    unset(full_lib_name)
    #if(SYSTEM.Windows)
        # 安装时自动复制 DLL
        #file(GLOB_RECURSE OPENCV_DLLS "${opencv_lib_path}/../bin/*.dll")
        #foreach(DLL ${OPENCV_DLLS})
            #install(FILES ${DLL} DESTINATION bin)
            #file(COPY ${DLL} DESTINATION ${CMAKE_BINARY_DIR}/Release)
        #endforeach()
    #elseif(SYSTEM.Linux)
        #
    #else
       # 
    #endif()
    
endmacro()