set(rapidjson_include_dir ${ROOT_PATH}/third_party/rapidjson/include)

# Function to configure RapidJSON includes for a target
function(target_link_rapidjson target_name)
    target_include_directories(${target_name}
        SYSTEM PRIVATE
            ${rapidjson_include_dir}
    )
endfunction()
