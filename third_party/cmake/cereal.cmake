# Cereal header-only library configuration
# Cereal is a C++11 library for serialization

# Set cereal include directory
set(cereal_include_dir ${ROOT_PATH}/third_party/cereal/include)

# Check if cereal directory exists
if(NOT EXISTS ${cereal_include_dir})
    message(FATAL_ERROR "Cereal include directory not found at ${cereal_include_dir}")
endif()

# Print status message for debugging
message(STATUS "Cereal include directory: ${cereal_include_dir}")

# Define macro for easy linking in other modules
macro(set_cereal_lib)
    message(STATUS "Cereal library configured successfully")
endmacro()

# Function to configure Cereal includes for a target
function(target_link_cereal target_name)
    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${cereal_include_dir}
    )
endfunction()
