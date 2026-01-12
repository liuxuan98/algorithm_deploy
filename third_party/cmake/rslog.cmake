# RSLog third party library configuration
# This file configures the RSLog library for the project

function(set_rslog_lib)
    set(RSLOG_ROOT_PATH ${ROOT_PATH}/third_party/RSLog)

    # Check if RSLog directory exists
    if(NOT EXISTS ${RSLOG_ROOT_PATH})
        message(FATAL_ERROR "RSLog directory not found at ${RSLOG_ROOT_PATH}")
    endif()

    # Set platform-specific paths for CMake config files
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(RSLOG_CMAKE_PATH ${RSLOG_ROOT_PATH}/Linux/lib/cmake/RSLogKit)
        set(RSLOG_INCLUDE_PATH ${RSLOG_ROOT_PATH}/Linux/include)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(RSLOG_CMAKE_PATH ${RSLOG_ROOT_PATH}/Windows/lib/cmake/RSLogKit)
        set(RSLOG_INCLUDE_PATH ${RSLOG_ROOT_PATH}/Windows/include)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(RSLOG_CMAKE_PATH ${RSLOG_ROOT_PATH}/Darwin/lib/cmake/RSLogKit)
        set(RSLOG_INCLUDE_PATH ${RSLOG_ROOT_PATH}/Darwin/include)
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
    endif()

    # Add RSLog CMake path to CMAKE_PREFIX_PATH
    list(APPEND CMAKE_PREFIX_PATH ${RSLOG_CMAKE_PATH})
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)

    # Use find_package to locate RSLogKit
    find_package(RSLogKit REQUIRED
        PATHS ${RSLOG_CMAKE_PATH}
        NO_DEFAULT_PATH
    )

    if(RSLogKit_FOUND)
        message(STATUS "RSLogKit found: version ${RSLOGKIT_VERSION}")

        # Set variables for parent scope for backward compatibility
        get_target_property(RSLOG_INCLUDE_DIRS RSLogKit::RSLogKit INTERFACE_INCLUDE_DIRECTORIES)
        get_target_property(RSLOG_LOCATION RSLogKit::RSLogKit IMPORTED_LOCATION_RELEASE)
        if(NOT RSLOG_LOCATION)
            get_target_property(RSLOG_LOCATION RSLogKit::RSLogKit IMPORTED_LOCATION)
        endif()

        # Export variables for backward compatibility
        set(rslog_target RSLogKit::RSLogKit PARENT_SCOPE)
        set(rslog_lib RSLogKit::RSLogKit PARENT_SCOPE)  # For backward compatibility
        set(rslog_include_dirs ${RSLOG_INCLUDE_PATH} PARENT_SCOPE)  # Use explicit path
        set(rslog_version ${RSLOGKIT_VERSION} PARENT_SCOPE)

        message(STATUS "RSLogKit configured using find_package:")
        message(STATUS "  Version: ${RSLOGKIT_VERSION}")
        message(STATUS "  Target: RSLogKit::RSLogKit")
        message(STATUS "  Include dirs: ${RSLOG_INCLUDE_PATH}")
        message(STATUS "  Library location: ${RSLOG_LOCATION}")

        if(RSLOGKIT_WITH_SPDLOG)
            message(STATUS "  Spdlog support: enabled")
        endif()
        if(RSLOGKIT_WITH_GLOG)
            message(STATUS "  Glog support: enabled")
        endif()
        if(RSLOGKIT_WITH_LOG4CPLUS)
            message(STATUS "  Log4cplus support: enabled")
        endif()
    else()
        message(FATAL_ERROR "RSLogKit not found. Please check the installation.")
    endif()

endfunction()

# Function to configure RSLog includes for a target
function(target_link_rslog target_name)
    set_rslog_lib()
    target_link_libraries(${target_name} PRIVATE RSLogKit::RSLogKit)
    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${rslog_include_dirs}
    )

    # Set RPATH for RSLog dynamic libraries
    if(UNIX AND NOT APPLE)
        set(RSLOG_LIB_PATH ${ROOT_PATH}/third_party/RSLog/Linux/lib)
        if(EXISTS ${RSLOG_LIB_PATH})
            # Get existing RPATH if any
            get_target_property(EXISTING_RPATH ${target_name} INSTALL_RPATH)
            if(EXISTING_RPATH)
                set(NEW_RPATH "${EXISTING_RPATH};${RSLOG_LIB_PATH}")
            else()
                set(NEW_RPATH "${RSLOG_LIB_PATH}")
            endif()

            set_target_properties(${target_name} PROPERTIES
                INSTALL_RPATH "${NEW_RPATH}"
                BUILD_WITH_INSTALL_RPATH TRUE
            )
            target_link_directories(${target_name} PRIVATE ${RSLOG_LIB_PATH})
            message(STATUS "RSLog RPATH set to: ${RSLOG_LIB_PATH}")
        endif()
    endif()
endfunction()
