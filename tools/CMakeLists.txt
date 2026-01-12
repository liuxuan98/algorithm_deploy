# Tools CMakeLists.txt
# 统一管理所有工具项目

# set tools work folder
set(TOOLS_WORK_FOLDER tools)

# tools options
option(ENABLE_PACK_MODELS_TOOL "Enable Pack Models Tool" ON)

# include zlib configuration for tools that need serialization
include(${ROOT_PATH}/third_party/cmake/zlib.cmake)

# Pass model compilation options to tools
if(ENABLE_MNN_MODEL)
    add_definitions(-DENABLE_MNN_MODEL)
    message(STATUS "Tools: MNN model support enabled")
endif()

if(ENABLE_OPENVINO_MODEL)
    add_definitions(-DENABLE_OPENVINO_MODEL)
    message(STATUS "Tools: OpenVINO model support enabled")
endif()

if(ENABLE_ONNX_MODEL)
    add_definitions(-DENABLE_ONNX_MODEL)
    message(STATUS "Tools: ONNX model support enabled")
endif()

# add pack_models tool
if(ENABLE_PACK_MODELS_TOOL)
    message(STATUS "Building pack_models tool...")
    add_subdirectory(pack_models)
endif()

# 未来可以在此添加其他工具
# if(ENABLE_OTHER_TOOL)
#     add_subdirectory(other_tool)
# endif()

# unset
unset(TOOLS_WORK_FOLDER)
