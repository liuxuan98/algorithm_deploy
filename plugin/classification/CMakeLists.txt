message(STATUS "plugin/classification_Thyroid_Breast")

# set
set(PLUGIN_SOURCE)
#set(PLUGIN_OBJECT)
set(PLUGIN_BINARY plugin_classification_Thyroid_Breast)

# SOURCE
file(GLOB PLUGIN_SOURCE
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)


# TARGET
add_library(${PLUGIN_BINARY} ${LIB_TYPE} ${PLUGIN_SOURCE})

# # DIRECTORY
set_property(TARGET ${PLUGIN_BINARY} PROPERTY FOLDER ${PLUGIN_DIRECTORY})

# # LINK_LIBRARIES

include(${ROOT_PATH}/third_party/cmake/opencv.cmake)
target_link_opencv(${PLUGIN_BINARY})


target_link_libraries(${PLUGIN_BINARY} PRIVATE ${LIB_NAME}) 

# # install
if(SYSTEM.Windows)
    #todo
else()

endif()

# append list
set(PLUGIN_LIST ${PLUGIN_LIST} ${PLUGIN_BINARY})

# unset
unset(PLUGIN_SOURCE)
#unset(PLUGIN_OBJECT)
unset(PLUGIN_BINARY)