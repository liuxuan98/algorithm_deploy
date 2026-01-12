# OpenVINO CMake configuration using find_package
# This configuration supports cross-platform deployment (Linux/Windows)

# Set CMake policy for find_package to use _ROOT variables
cmake_policy(SET CMP0074 NEW)

# Set the root path if not already defined
if(NOT ROOT_PATH)
    set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()

# Set platform-specific OpenVINO directory and architecture
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OpenVINO_ROOT ${ROOT_PATH}/third_party/openvino/Linux/x86_64/runtime)
    set(OPENVINO_CMAKE_DIR ${OpenVINO_ROOT}/cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OpenVINO_ROOT ${ROOT_PATH}/third_party/openvino/Windows/AMD64)
    set(OPENVINO_CMAKE_DIR ${OpenVINO_ROOT}/cmake)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Debug information
message(STATUS "OpenVINO platform directory: ${OpenVINO_ROOT}")
message(STATUS "OpenVINO CMake directory: ${OPENVINO_CMAKE_DIR}")

# Verify that the OpenVINO installation exists
if(NOT EXISTS ${OPENVINO_CMAKE_DIR}/OpenVINOConfig.cmake)
    message(FATAL_ERROR "OpenVINO installation not found at: ${OpenVINO_ROOT}")
endif()

# Use find_package to locate OpenVINO
find_package(OpenVINO REQUIRED
    PATHS ${OPENVINO_CMAKE_DIR}
    NO_DEFAULT_PATH
)

if(OpenVINO_FOUND)
    message(STATUS "OpenVINO found successfully!")
    message(STATUS "OpenVINO version: ${OpenVINO_VERSION}")

    # Include directories
    set(openvino_include_dir ${OpenVINO_ROOT}/include)

    # Log the available targets
    message(STATUS "OpenVINO Runtime target: openvino::runtime")

    # Create a macro for backward compatibility with existing build system
    macro(set_openvino_lib)
        # Use the modern OpenVINO targets
        set(openvino_lib openvino::runtime)
        set(openvino_include_dir ${OpenVINO_ROOT}/include)

        message(STATUS "OpenVINO libraries configured using modern CMake targets")
    endmacro()

    # Function to configure OpenVINO includes for a target
    function(target_link_openvino target_name)
        target_link_libraries(${target_name} PRIVATE openvino::runtime)
        target_include_directories(${target_name}
            SYSTEM PRIVATE
                ${openvino_include_dir}
                ${openvino_include_dir}/ie
        )

        # Set RPATH for OpenVINO dynamic libraries
        if(UNIX AND NOT APPLE)
            set(OPENVINO_LIB_PATH ${OpenVINO_ROOT}/lib/intel64)
            if(EXISTS ${OPENVINO_LIB_PATH})
                # Get existing RPATH if any
                get_target_property(EXISTING_RPATH ${target_name} INSTALL_RPATH)
                if(EXISTING_RPATH)
                    set(NEW_RPATH "${EXISTING_RPATH};${OPENVINO_LIB_PATH}")
                else()
                    set(NEW_RPATH "${OPENVINO_LIB_PATH}")
                endif()

                set_target_properties(${target_name} PROPERTIES
                    INSTALL_RPATH "${NEW_RPATH}"
                    BUILD_WITH_INSTALL_RPATH TRUE
                )
                target_link_directories(${target_name} PRIVATE ${OPENVINO_LIB_PATH})
                message(STATUS "OpenVINO RPATH set to: ${OPENVINO_LIB_PATH}")
            endif()
        endif()
    endfunction()

else()
    message(FATAL_ERROR "OpenVINO not found in ${OPENVINO_CMAKE_DIR}")
endif()
