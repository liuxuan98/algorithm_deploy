#add required plugin

#make 
set(PLUGIN_DIRECTORY rs_plugin)
set(PLUGIN_LIST)

if(ENABLE_CLASSIFICATION_PLUGIN)
    add_subdirectory(classification)
endif()