# Pack Models Tool CMakeLists.txt
# 模型序列化打包工具构建配置 - 独立构建，支持所有模型类型

cmake_minimum_required(VERSION 3.16)

project(pack_models LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cross-platform compatibility
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# Enable all model types for pack_models tool
if(ENABLE_MNN_MODEL)
    add_definitions(-DENABLE_MNN_MODEL)
endif()
if(ENABLE_OPENVINO_MODEL)
    add_definitions(-DENABLE_OPENVINO_MODEL)
endif()
if(ENABLE_ONNX_MODEL)
    add_definitions(-DENABLE_ONNX_MODEL)
endif()

# Include cereal library
include(${ROOT_PATH}/third_party/cmake/cereal.cmake)
set_cereal_lib()

# Include zlib for compression
include(${ROOT_PATH}/third_party/cmake/zlib.cmake)

# Include RSLog for logging support
include(${ROOT_PATH}/third_party/cmake/rslog.cmake)
set_rslog_lib()

# Include CryptoPP for encryption support
include(${ROOT_PATH}/third_party/cmake/cryptopp.cmake)
check_cryptopp_available(CRYPTOPP_AVAILABLE)
if(CRYPTOPP_AVAILABLE)
    set_cryptopp_lib()
    add_definitions(-DENABLE_CRYPTOPP)
    message(STATUS "Pack Models: CryptoPP encryption support enabled")
else()
    message(WARNING "Pack Models: CryptoPP not available, encryption features will be disabled")
endif()

# Pack Models Tool source files
set(PACK_MODELS_SOURCES
    src/main.cpp
    src/model_serializer.cpp
    src/model_packager.cpp
    src/cli_parser.cpp
    src/utils/file_utils.cpp
    src/utils/platform_utils.cpp
    src/utils/model_parse_crypto.cpp
)

# Header files
set(PACK_MODELS_HEADERS
    include/model_serializer.h
    include/model_packager.h
    include/cli_parser.h
    include/utils/file_utils.h
    include/utils/platform_utils.h
    include/utils/model_parse_crypto.h
)

# Create executable with pack_models sources
add_executable(pack_models
    ${PACK_MODELS_SOURCES}
    ${PACK_MODELS_HEADERS}
)

# include directories
target_include_directories(pack_models
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${ROOT_PATH}/kernel/include
)

# Configure third-party libraries
target_link_cereal(pack_models)
target_link_zlib(pack_models)
target_link_rslog(pack_models)

# Configure CryptoPP
if(CRYPTOPP_AVAILABLE)
    target_link_cryptopp(pack_models)
endif()

# Link libraries
target_link_libraries(pack_models
    PRIVATE
        rs_core
        ${ZLIB_LIBRARIES}
        ${rslog_lib}
)

# Add CryptoPP library if available
if(CRYPTOPP_AVAILABLE AND cryptopp_lib)
    target_link_libraries(pack_models PRIVATE ${cryptopp_lib})
    message(STATUS "Pack Models: Linking with CryptoPP library: ${cryptopp_lib}")
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(pack_models
        PRIVATE
            ws2_32
            mswsock
    )
endif()

if(UNIX)
    target_link_libraries(pack_models
        PRIVATE
            pthread
            dl
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(pack_models PRIVATE /W4)
else()
    target_compile_options(pack_models PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set output directory
set_target_properties(pack_models PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install target
install(TARGETS pack_models
    RUNTIME DESTINATION bin
)

message(STATUS "Pack Models Tool configured for ${CMAKE_SYSTEM_NAME}")
