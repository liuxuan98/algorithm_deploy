# 根据平台进行不同平台的编译优化,库的前缀 后缀等配置
# system and process
# 一些公共链接三方库的宏 ,根据平台配置对应的cmake编译选项 调优
set(LIB_PREFIX "lib")
set(LIB_SUFFIX ".so") # unix or linux
if(CMAKE_SYSTEM_NAME MATCHES "^Android")
    set(SYSTEM.Android 1)
elseif(CMAKE_SYSTEM_NAME MATCHES "^Linux")
    set(SYSTEM.Linux 1)
elseif(CMAKE_SYSTEM_NAME MATCHES "^Windows")
    set(SYSTEM.Windows 1)
    set(LIB_PREFIX "")
    set(LIB_SUFFIX ".lib")
elseif(CMAKE_SYSTEM_NAME MATCHES "^Darwin")
    set(SYSTEM.Darwin 1)
    set(LIB_SUFFIX ".dylib")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(PROCESSOR.arm 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
    set(PROCESSOR.aarch64 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86")
    set(PROCESSOR.x86 1)
endif()

# android
if(SYSTEM.Android)

    if(PROCESSOR.arm)
        add_definitions(-mfloat-abi=softfp -mfpu=neon)
    endif()
    #todo
endif()

#ios
if(APPLE)
    #if() 配置一些cmake的条件


    
    if(PROCESSOR.arm)
        add_definitions(-mfloat-abi=softfp -mfpu=neon )
    endif()
endif()

#mac

#linux
if(SYSTEM.Linux)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STRICT_ANSI__")

    #if()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        #this is to workaround libgcc.a
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()

    if(PROCESSOR.arm)
        add_definitions(-mfpu=neon)
    endif()

    if(PROCESSOR.arm OR PROCESSOR.aarch64)
        set(aarch64_linux_include
            # /user/inlcude/c++/4.9
            # /user/lib/gcc/x86_64-linux-gnu/4.9
        )
        include_directories(${aarch64_linux_include})
    endif()
endif()
#windows
## ignore deprecated warning
if(SYSTEM.Windows)
    if(ENABLE_BUILD_SHARED)
        add_definitions(-DBUILDING_RS_DLL)#fuhao导出 window and build shared lib
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wno-ignored-attributes")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-ignored-attributes")  
endif()

#debug or release
if(CMAKE_BUILD_TYPE MATCHES Debug)
    #set()
    #add_definitions(-DDEBUG) ## notuds


    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DEBUG")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
    endif()
else()
    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /Zi") #/ zi
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

        if(SYSTEM.Android)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -fPIE")
            set(CMAKE_SHARED_LINKER_FLAGS "")
            #...
            #设置安卓的环境
        endif()
    endif()
endif(CMAKE_BUILD_TYPE MATCHES Debug)

#windows 也有隐藏符号的概念
# # symbol hidden //mnn
if((NOT MSVC) AND ENABLE_SYMBOL_HIDE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden") 

    #Omit frame pointer may cause difficult debug
    if((NOT APPLE) AND (NOT WIN32))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer")
    endif()
endif()

## cxx_11 //tnn
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    if(RS_DEPLOY_GLIBCXX_USE_CXX14_ABI_ENABLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX14_ABI=1")
    elseif(RS_DEPLOY_GLIBCXX_USE_CXX11_ABI_ENABLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    endif()
endif()



# # c/c++ version
set(CMAKE_C_STANDARD 99)

if(ENABLE_GLIBCXX_USE_CXX17_ABI)
    set(CMAKE_CXX_STANDARD 17)
elseif(ENABLE_GLIBCXX_USE_CXX14_ABI)
    set(CMAKE_CXX_STANDARD 14)
elseif(ENABLE_GLIBCXX_USE_CXX11_ABI)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# #coverage 覆盖率分析（Coverage Analysis）是一种用于评估程序测试质量的技术，它通过检测代码在执行过程中哪些部分被访问到，哪些没有被执行，从而帮助开发者提高测试的完备性。
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -coverage -fprofile-arcs -ftest-coverage")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -coverage -lgcov")
    endif()
endif()

# # sanitize
if(ENABLE_ADDRESS_SANTIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

# # shared library
if(ENABLE_BUILD_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

# # postfix
set(CMAKE_BUILD_POSTFIX "")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRW_LIBRARY_POST=${}")

# define
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DNNDEPLOY_DEBUG)
endif()

set(CMAKE_DEPENDS_IN_PROJECT_ONLY ON)

# target path
set(TARGET_RUN_DIR ${ROOT_PATH}/build/target/${CMAKE_SYSTEM_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/lib)

if(SYSTEM.Windows)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/bin)
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/pdb)
elseif(SYSTEM.Darwin)  ## cmake 语法嵌套混乱
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/bin)
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/pdb)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    cmake_policy(SET CMP0068 OLD) ##
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@executable_path")
elseif(SYSTEM.Linux)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/lib)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "\${ORIGIN}/")
else() 
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_RUN_DIR}/bin)
endif()

link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}) ##命令告诉连接器在编译时到指定目录中查找库文件(.a .so等) 动态库目录和静态库目录

## 安装pdb目录和include目录
set(INSTALL_RUN_DIR ${CMAKE_INSTALL_PREFIX}) ## not use del
set(include_file_dir "${ROOT_PATH}/include/") ##ROOT_PATH 为当前工程根目录

# 如果使用cpack，install的DESTINATION 不要写绝对路径，不然会打进包里 ,相对路径
install(DIRECTORY ${include_file_dir} DESTINATION include)  


if(SYSTEM.Windows OR SYSTEM.Darwin)
    set(pdb_file_dir "${TARGET_RUN_DIR}/pdb/${CMAKE_BUILD_TYPE}")
    install(DIRECTORY ${pdb_file_dir} DESTINATION pdb)
endif()

## 设置查找第三方库的路径为both，表示支持自定义路径和系统路径，不然android下find_library无法找到自定义的第三方库
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH) #交叉编译

#用于设置全局依赖，只限定系统自带库，比如所有的android都依赖log

set(global_all_platform_system_lib)
message(STATUS "\tgloal_all_platform_system_lib:" ${global_all_platform_system_lib})


if(SYSTEM.Android)
    find_library(android_lib log)   ## 安卓的日志功能
    list(APPEND global_all_platform_system_lib ${android_lib})
    list(APPEND global_all_platform_system_lib -landroid -lz -lGLESv3 -lEGL -ljnigraphics -lmediandk)
elseif(SYSTEM.Linux)
    list(APPEND global_all_platform_system_lib -lm -lz) ## linux的数学库 和压缩库
elseif(SYSTEM.Windows)
    list(APPEND global_all_platform_system_lib)

elseif(SYSTEM.Wasm)# web 浏览器
    ##
endif()

message(STATUS "\tgloal_all_platform_system_lib:" ${global_all_platform_system_lib})

# 设置第三方库及其依赖库的拷入
set(third_lib_all_copy_dir_list)
set(third_lib_all_copy_file_list)
set(copy_target_path)
set(copy_install_path)

if(SYSTEM.Windows)
    if(ENABLE_3RD_OPENVINO)
        list(APPEND third_lib_all_copy_dir_list ${ROOT_PATH}/third_party/openvino/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/bin/${CMAKE_BUILD_TYPE})
    endif()

    if(ENABLE_3RD_OPENCV)
        list(APPEND third_lib_all_copy_dir_list ${ROOT_PATH}/third_party/opencv/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/bin/${CMAKE_BUILD_TYPE})
    endif()

    if(ENABLE_3RD_NCNN)
        list(APPEND third_lib_all_copy_dir_list ${ROOT_PATH}/third_party/ncnn/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/bin/${CMAKE_BUILD_TYPE})
    endif()

    ## Raidjson 系三方库

elseif(SYSTEM.Linux)
    if(ENABLE_3RD_TENSORRT)
        list(APPEND third_lib_all_copy_dir_list ${ROOT_PATH}/third_party/tensorrt/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/bin/${CMAKE_BUILD_TYPE})
    endif()

endif()




if(SYSTEM.Windows)
    set(copy_target_path "${TARGET_RUN_DIR}/bin/CMAKE_BUILD_TYPE")  ## target_path 中所有文件需要打包
    set(copy_install_path "bin/${CMAKE_BUILD_TYPE}")
    if(NOT PROCESS.arm)
        list(APPEND third_lib_all_copy_dir_list ${ROOT_PATH}/third_party/system_lib/Windows/Release)
    endif() ## windows 的系统库 vs的依赖的系统库
elseif(SYSTEM.Darwin)
    #if(NOT DEFINDED )

elseif(SYSTEM.iOS)
    set(copy_target_path "${TARGET_RUN_DIR}/bin/${CMAKE_BUILD_TYPE}")
    set(copy_install_path "lib/${CMAKE_BUILD_TYPE}")
elseif(SYSTEM.Android)
    set(copy_target_path "${TARGET_RUN_DIR}/bin/${CMAKE_BUILD_TYPE}")
    set(copy_install_path "lib/${CMAKE_BUILD_TYPE}")
elseif(SYSTEM.Linux)
    set(copy_target_path "${TARGET_RUN_DIR}/bin/${CMAKE_BUILD_TYPE}")
    set(copy_install_path "lib/${CMAKE_BUILD_TYPE}")
elseif(SYSTEM.Wasm)
    set(copy_target_path "${TARGET_RUN_DIR}/lib")
    set(copy_install_path "lib")
endif()

foreach(third_lib_path IN LISTS third_lib_all_copy_dir_list)

    file(GLOB third_lib_files "${third_lib_path}/*")
    install(FILES ${third_lib_files} DESTINATION "${copy_target_path}")
    # 如果使用cpack，install的DESTINATION 不要写绝对路径，不然会打进包里
    install(FILES ${third_lib_file} DESTINATION "${copy_install_path}")
endforeach()

foreach(third_lib_file IN LISTS third_lib_all_copy_file_list)
    install(FILES ${third_lib_file} DESTINATION "${copy_target_path}")

    install(FILES ${third_lib_file} DESTINATION "${copy_install_path}")
endforeach()
# 安装linsence文件
#install() 略

if(SYSTEM.Wasm)
# web 浏览器
endif()

#cpack
include(${ROOT_PATH}/cmake/common/cpack.cmake)


if(RS_DEPOLY_ENABLE_DEVICE_X86)
    add_definitions(-DRS_DEPOLY_ENABLE_DEVICE_X86)
elseif(RS_DEPOLY_ENABLE_DEVICE_ARM)
    add_definitions(-DRS_DEPOLY_ENABLE_DEVICE_ARM)
endif()

if(APPLE)
    ##
    ##
endif()
    


