# ZLib CMake configuration
# ZLib is a compression library required for CRC32 calculations

# Set the third-party zlib root path
if(DEFINED ROOT_PATH)
    set(ZLIB_ROOT_PATH ${ROOT_PATH}/third_party/zlib)
else()
    set(ZLIB_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zlib)
endif()

# Check platform and set appropriate paths
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit build
        set(ZLIB_PLATFORM_PATH ${ZLIB_ROOT_PATH}/Windows/AMD64)
    else()
        # 32-bit build - you may need to add x86 libraries if needed
        message(FATAL_ERROR "32-bit Windows build not supported. Please add x86 zlib libraries.")
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux platform - use system zlib
    find_package(ZLIB REQUIRED)
    if(ZLIB_FOUND)
        set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
        set(ZLIB_LIBRARIES ${ZLIB_LIBRARIES})
        message(STATUS "Using system zlib on Linux: ${ZLIB_LIBRARIES}")

        # Define the function for system zlib
        function(target_link_zlib target_name)
            target_link_libraries(${target_name} PRIVATE ${ZLIB_LIBRARIES})
            target_include_directories(${target_name}
                SYSTEM PRIVATE
                    ${ZLIB_INCLUDE_DIRS}
            )
        endfunction()

        return() # Early return since we're using system zlib
    else()
        message(FATAL_ERROR "System zlib not found on Linux. Please install zlib development package.")
    endif()
elseif(APPLE)
    # macOS platform - you can add macOS libraries later if needed
    message(FATAL_ERROR "macOS platform not supported yet. Please add macOS zlib libraries.")
endif()

# Set zlib paths
set(ZLIB_INCLUDE_DIR ${ZLIB_PLATFORM_PATH}/include)
set(ZLIB_LIBRARY_RELEASE ${ZLIB_PLATFORM_PATH}/lib/zlib.lib)
set(ZLIB_LIBRARY_DEBUG ${ZLIB_PLATFORM_PATH}/lib/zlib.lib)

# Check if files exist
if(NOT EXISTS ${ZLIB_INCLUDE_DIR}/zlib.h)
    message(FATAL_ERROR "ZLib header not found at: ${ZLIB_INCLUDE_DIR}/zlib.h")
endif()

if(NOT EXISTS ${ZLIB_LIBRARY_RELEASE})
    message(FATAL_ERROR "ZLib library not found at: ${ZLIB_LIBRARY_RELEASE}")
endif()

# Set zlib variables for compatibility
set(ZLIB_FOUND TRUE)
set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
set(ZLIB_LIBRARIES ${ZLIB_LIBRARY_RELEASE})

if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
    set_target_properties(ZLIB::ZLIB PROPERTIES
        IMPORTED_LOCATION ${ZLIB_LIBRARY_RELEASE}
        INTERFACE_INCLUDE_DIRECTORIES ${ZLIB_INCLUDE_DIR}
    )
endif()

# Print status message for debugging
message(STATUS "ZLib found: ${ZLIB_LIBRARIES}")
message(STATUS "ZLib include directories: ${ZLIB_INCLUDE_DIRS}")
message(STATUS "ZLib platform path: ${ZLIB_PLATFORM_PATH}")

# Function to configure ZLib includes for a target
function(target_link_zlib target_name)
    if(TARGET ZLIB::ZLIB)
        target_link_libraries(${target_name} PRIVATE ZLIB::ZLIB)
    else()
        target_link_libraries(${target_name} PRIVATE ${ZLIB_LIBRARIES})
        target_include_directories(${target_name}
            SYSTEM PRIVATE
                ${ZLIB_INCLUDE_DIRS}
        )
    endif()
endfunction()
