# 宏函数：自动配置 gtest 头文件与库文件

macro(set_gtest_lib ARG0)

    set(gtest_lib)

    set(libs "gtest" "gtest_main") #only link gtest lib

    #include
    set(gtest_include_dir ${ROOT_PATH}/third_party/googletest/include)

    set(gtest_lib_path ${ROOT_PATH}/third_party/googletest/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/${CMAKE_BUILD_TYPE})

    foreach(lib ${libs})
        set(lib_name ${LIB_PREFIX}${lib}${LIB_SUFFIX})
        set(full_lib_name ${gtest_lib_path}/${lib_name})
        list(APPEND gtest_lib ${full_lib_name})
    endforeach()

    unset(lib_name)
    unset(full_lib_name)
    unset(gtest_lib_path)
    unset(gtest_include_dir)
    unset(libs)

endmacro()

# Function to configure GoogleTest includes for a target
function(target_link_gtest target_name)
    set_gtest_lib("")
    target_link_libraries(${target_name} PRIVATE ${gtest_lib})
    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${ROOT_PATH}/third_party/googletest/include
    )
endfunction()
