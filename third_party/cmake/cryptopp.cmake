# CryptoPP library configuration
# Set CryptoPP library paths and include directories

# Function to detect Visual Studio version and set appropriate vc directory
function(detect_vs_version vs_version_dir)
    if(MSVC)
        # Get MSVC version
        if(MSVC_VERSION GREATER_EQUAL 1930)
            # Visual Studio 2022 (version 17.x) - use vc17
            set(${vs_version_dir} "vc17" PARENT_SCOPE)
            message(STATUS "Detected Visual Studio 2022 (${MSVC_VERSION}), using vc17")
        elseif(MSVC_VERSION GREATER_EQUAL 1920)
            # Visual Studio 2019 (version 16.x) - use vc16
            set(${vs_version_dir} "vc16" PARENT_SCOPE)
            message(STATUS "Detected Visual Studio 2019 (${MSVC_VERSION}), using vc16")
        elseif(MSVC_VERSION GREATER_EQUAL 1910)
            # Visual Studio 2017 (version 15.x) - use vc16
            set(${vs_version_dir} "vc16" PARENT_SCOPE)
            message(STATUS "Detected Visual Studio 2017 (${MSVC_VERSION}), using vc16")
        else()
            # Fallback to vc16 for older versions
            set(${vs_version_dir} "vc16" PARENT_SCOPE)
            message(WARNING "Unknown Visual Studio version (${MSVC_VERSION}), defaulting to vc16")
        endif()
    else()
        # Non-MSVC compiler, default to vc17 (most recent)
        set(${vs_version_dir} "vc17" PARENT_SCOPE)
        message(STATUS "Non-MSVC compiler detected, defaulting to vc17")
    endif()
endfunction()

if(WIN32)
    # Detect Visual Studio version and select appropriate directory
    detect_vs_version(VS_VERSION_DIR)
    
    set(CRYPTOPP_ROOT_PATH ${ROOT_PATH}/third_party/cryptopp/Windows/AMD64/${VS_VERSION_DIR})
    set(CRYPTOPP_INCLUDE_PATH ${CRYPTOPP_ROOT_PATH}/include)
    set(CRYPTOPP_LIB_PATH ${CRYPTOPP_ROOT_PATH}/lib)
    set(CRYPTOPP_LIB_NAME cryptopp)
elseif(UNIX AND NOT APPLE)
    # Linux
    set(CRYPTOPP_ROOT_PATH ${ROOT_PATH}/third_party/cryptopp/Linux/x86_64)
    set(CRYPTOPP_INCLUDE_PATH ${CRYPTOPP_ROOT_PATH}/include)
    set(CRYPTOPP_LIB_PATH ${CRYPTOPP_ROOT_PATH}/lib)
    set(CRYPTOPP_LIB_NAME cryptopp)
elseif(APPLE)
    # macOS (when available)
    set(CRYPTOPP_ROOT_PATH ${ROOT_PATH}/third_party/cryptopp/macOS)
    set(CRYPTOPP_INCLUDE_PATH ${CRYPTOPP_ROOT_PATH}/include)
    set(CRYPTOPP_LIB_PATH ${CRYPTOPP_ROOT_PATH}/lib)
    set(CRYPTOPP_LIB_NAME cryptopp)
endif()

function(set_cryptopp_lib)
    if(EXISTS ${CRYPTOPP_INCLUDE_PATH} AND EXISTS ${CRYPTOPP_LIB_PATH})

        # Library paths
        link_directories(${CRYPTOPP_LIB_PATH})

        # Set library variable - always use the full path to be safe
        if(WIN32)
            set(cryptopp_lib ${CRYPTOPP_LIB_PATH}/cryptopp.lib PARENT_SCOPE)
        else()
            set(cryptopp_lib ${CRYPTOPP_LIB_PATH}/libcryptopp.a PARENT_SCOPE)
        endif()

        message(STATUS "CryptoPP found: ${CRYPTOPP_ROOT_PATH}")
        message(STATUS "CryptoPP include: ${CRYPTOPP_INCLUDE_PATH}")
        message(STATUS "CryptoPP lib path: ${CRYPTOPP_LIB_PATH}")
        message(STATUS "CryptoPP lib name: ${cryptopp_lib}")
    else()
        message(WARNING "CryptoPP not found at: ${CRYPTOPP_ROOT_PATH}")
        
        # On Windows, try alternative version if current one doesn't exist
        if(WIN32)
            if(VS_VERSION_DIR STREQUAL "vc16")
                message(STATUS "Trying alternative version vc17...")
                set(ALT_CRYPTOPP_ROOT_PATH ${ROOT_PATH}/third_party/cryptopp/Windows/AMD64/vc17)
            else()
                message(STATUS "Trying alternative version vc16...")
                set(ALT_CRYPTOPP_ROOT_PATH ${ROOT_PATH}/third_party/cryptopp/Windows/AMD64/vc16)
            endif()
            
            set(ALT_CRYPTOPP_INCLUDE_PATH ${ALT_CRYPTOPP_ROOT_PATH}/include)
            set(ALT_CRYPTOPP_LIB_PATH ${ALT_CRYPTOPP_ROOT_PATH}/lib)
            
            if(EXISTS ${ALT_CRYPTOPP_INCLUDE_PATH} AND EXISTS ${ALT_CRYPTOPP_LIB_PATH})
                # Update paths to use alternative version
                set(CRYPTOPP_ROOT_PATH ${ALT_CRYPTOPP_ROOT_PATH} PARENT_SCOPE)
                set(CRYPTOPP_INCLUDE_PATH ${ALT_CRYPTOPP_INCLUDE_PATH} PARENT_SCOPE)
                set(CRYPTOPP_LIB_PATH ${ALT_CRYPTOPP_LIB_PATH} PARENT_SCOPE)
                
                link_directories(${ALT_CRYPTOPP_LIB_PATH})
                set(cryptopp_lib ${ALT_CRYPTOPP_LIB_PATH}/cryptopp.lib PARENT_SCOPE)
                
                message(STATUS "CryptoPP found (alternative): ${ALT_CRYPTOPP_ROOT_PATH}")
                message(STATUS "CryptoPP include (alternative): ${ALT_CRYPTOPP_INCLUDE_PATH}")
                message(STATUS "CryptoPP lib path (alternative): ${ALT_CRYPTOPP_LIB_PATH}")
            else()
                message(FATAL_ERROR "CryptoPP not found in either vc16 or vc17 directories!")
                set(cryptopp_lib "" PARENT_SCOPE)
            endif()
        else()
            set(cryptopp_lib "" PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Check if CryptoPP is available
function(check_cryptopp_available result_var)
    if(EXISTS ${CRYPTOPP_INCLUDE_PATH} AND EXISTS ${CRYPTOPP_LIB_PATH})
        set(${result_var} TRUE PARENT_SCOPE)
    else()
        # On Windows, try alternative version if current one doesn't exist
        if(WIN32)
            if(VS_VERSION_DIR STREQUAL "vc16")
                set(ALT_CRYPTOPP_ROOT_PATH ${ROOT_PATH}/third_party/cryptopp/Windows/AMD64/vc17)
            else()
                set(ALT_CRYPTOPP_ROOT_PATH ${ROOT_PATH}/third_party/cryptopp/Windows/AMD64/vc16)
            endif()
            
            set(ALT_CRYPTOPP_INCLUDE_PATH ${ALT_CRYPTOPP_ROOT_PATH}/include)
            set(ALT_CRYPTOPP_LIB_PATH ${ALT_CRYPTOPP_ROOT_PATH}/lib)
            
            if(EXISTS ${ALT_CRYPTOPP_INCLUDE_PATH} AND EXISTS ${ALT_CRYPTOPP_LIB_PATH})
                set(${result_var} TRUE PARENT_SCOPE)
            else()
                set(${result_var} FALSE PARENT_SCOPE)
            endif()
        else()
            set(${result_var} FALSE PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Function to configure CryptoPP includes for a target
function(target_link_cryptopp target_name)
    set_cryptopp_lib()
    if(cryptopp_lib)
        target_link_libraries(${target_name} PRIVATE ${cryptopp_lib})
        target_include_directories(${target_name}
            SYSTEM PRIVATE
                ${CRYPTOPP_INCLUDE_PATH}
        )
    endif()
endfunction()
