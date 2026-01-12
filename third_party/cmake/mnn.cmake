# MNN cmake configuration
# Set MNN library paths and include directories

function(set_mnn_lib)
    if(WIN32)
        # Windows configuration
        set(MNN_ROOT_PATH ${ROOT_PATH}/third_party/mnn/Windows/AMD64)
        set(MNN_INCLUDE_PATH ${MNN_ROOT_PATH}/include)
        set(MNN_LIB_PATH ${MNN_ROOT_PATH}/lib)

        # MNN libraries for Windows
        set(mnn_lib ${MNN_LIB_PATH}/MNN.lib PARENT_SCOPE)
        set(MNN_INCLUDE_PATH ${MNN_INCLUDE_PATH} PARENT_SCOPE)
        set(MNN_LIB_PATH ${MNN_LIB_PATH} PARENT_SCOPE)

    elseif(UNIX AND NOT APPLE)
        # Linux configuration
        set(MNN_ROOT_PATH ${ROOT_PATH}/third_party/mnn/Linux/x86_64)
        set(MNN_INCLUDE_PATH ${MNN_ROOT_PATH}/include)
        set(MNN_LIB_PATH ${MNN_ROOT_PATH}/lib)

        # Add library directory to link directories
        link_directories(${MNN_LIB_PATH})

        # MNN libraries for Linux
        set(mnn_lib ${MNN_LIB_PATH}/libMNN.so ${MNN_LIB_PATH}/libMNN_Express.so PARENT_SCOPE)
        set(MNN_INCLUDE_PATH ${MNN_INCLUDE_PATH} PARENT_SCOPE)
        set(MNN_LIB_PATH ${MNN_LIB_PATH} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "MNN: Unsupported platform")
    endif()

    message(STATUS "MNN library path: ${MNN_LIB_PATH}")
    message(STATUS "MNN include path: ${MNN_INCLUDE_PATH}")
endfunction()

# Function to configure MNN includes for a target
function(target_link_mnn target_name)
    set_mnn_lib()
    
    target_link_libraries(${target_name} PRIVATE ${mnn_lib})

    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${MNN_INCLUDE_PATH}
    )
    
    message(STATUS "MNN include path added to target ${target_name}: ${MNN_INCLUDE_PATH}")
    message(STATUS "MNN libraries linked: ${mnn_lib}")

    if(UNIX AND NOT APPLE)
        # Get existing RPATH if any
        get_target_property(EXISTING_RPATH ${target_name} INSTALL_RPATH)
        if(EXISTING_RPATH)
            set(NEW_RPATH "${EXISTING_RPATH};${MNN_LIB_PATH}")
        else()
            set(NEW_RPATH "${MNN_LIB_PATH}")
        endif()

        set_target_properties(${target_name} PROPERTIES
            INSTALL_RPATH "${NEW_RPATH}"
            BUILD_WITH_INSTALL_RPATH TRUE
        )

        target_link_directories(${target_name} PRIVATE ${MNN_LIB_PATH})
        message(STATUS "MNN RPATH set to: ${MNN_LIB_PATH}")
    endif()
endfunction()
