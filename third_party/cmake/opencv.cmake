# 定义宏：自动配置 OpenCV 头文件与库
macro(set_opencv_lib ARG0)

    message(STATUS "set_opencv_lib: ${ARG0}")

    # 根据平台设置不同的库名
    if(CMAKE_SYSTEM_NAME MATCHES "^Windows")
        # Windows 平台库名
        if("${ARG0}" STREQUAL "")
            set(libs "core452" "imgproc452" "highgui452") ## 非world的cv库，核心链接的三个库
        elseif("${ARG0}" STREQUAL "world")
            set(libs "opencv_world452")
        endif()
    elseif(CMAKE_SYSTEM_NAME MATCHES "^Linux")
        # Linux 平台库名 (Linux上只有world版本)
        if("${ARG0}" STREQUAL "")
            # Linux上使用world版本代替分离的库
            set(libs "opencv_world")
        elseif("${ARG0}" STREQUAL "world")
            set(libs "opencv_world")
        endif()
    else()
        # 其他平台默认使用Windows命名
        if("${ARG0}" STREQUAL "")
            set(libs "core452" "imgproc452" "highgui452")
        elseif("${ARG0}" STREQUAL "world")
            set(libs "opencv_world452")
        endif()
    endif()

    set(opencv_lib)
    #包含头文件
    set(opencv_include_dir ${ROOT_PATH}/third_party/opencv/include)

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

    # # 平台特定的安装配置
    # if(CMAKE_SYSTEM_NAME MATCHES "^Windows")
    #     # Windows平台：安装时自动复制 DLL
    #     file(GLOB_RECURSE OPENCV_DLLS "${opencv_lib_path}/../bin/*.dll")
    #     foreach(DLL ${OPENCV_DLLS})
    #         install(FILES ${DLL} DESTINATION bin)
    #     endforeach()
    # elseif(CMAKE_SYSTEM_NAME MATCHES "^Linux")
    #     # Linux平台：安装时自动复制 SO 文件
    #     file(GLOB_RECURSE OPENCV_SOS "${opencv_lib_path}/*.so*")
    #     foreach(SO ${OPENCV_SOS})
    #         install(FILES ${SO} DESTINATION lib)
    #     endforeach()
    # endif()

endmacro()

# Function to configure OpenCV includes for a target
function(target_link_opencv target_name)
    # Call the existing macro to set up opencv_lib
    set_opencv_lib("world")

    target_link_libraries(${target_name} PRIVATE ${opencv_lib})
    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${ROOT_PATH}/third_party/opencv/include
    )
endfunction()
