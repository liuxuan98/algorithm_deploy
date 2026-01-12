#set
set(SOURCE)
set(OBJECT)
set(BINARY dag_classification_sample)
set(TARGET_DIRECTORY sample)
set(DEPEND_LIBRARY)
set(SYSTEM_LIBRARY)
set(THIRD_PARTY_LIBRARY)
set(PLUGIN_LIBRARY plugin_classification_Thyroid_Breast)

# source
file(GLOB SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
# define make demo use this API.
add_definitions(-DENABLE_3RD_OPENCV)

# lib dll import
# if(SYSTEM.Windows)
#     add_definitions(-DUSING_RS_DLL)
#     remove_definitions(-DBUILDING_RS_DLL) #防止sample demo的导出
# endif()

# binary
add_executable(${BINARY} ${SOURCE} ${OBJECT})

# target_directory
set_property(TARGET ${BINARY} PROPERTY FOLDER ${TARGET_DIRECTORY})
#link library
target_link_libraries(${BINARY} PRIVATE ${LIB_NAME})

# include: plugin_classification_Thyroid_Breast include
target_include_directories(${BINARY} PRIVATE ${ROOT_PATH}/plugin)

# depend_library :classification_plugin
find_library(CLS_PLUGIN_LIB plugin_classification_Thyroid_Breast PATHS ${PROJECT_BINARY_DIR}/target/${CMAKE_SYSTEM_NAME}/lib/Release)
if(NOT CLS_PLUGIN_LIB)
    message(FATAL_ERROR "error: no fine CLS_PLUGIN_LIB lib. please check the path is correct.")
endif()
message("CLS_PLUGIN_LIB: ${CLS_PLUGIN_LIB}")
list(APPEND DEPEND_LIBRARY ${CLS_PLUGIN_LIB})
target_link_libraries(${BINARY} PRIVATE ${DEPEND_LIBRARY})


# third_party_library
include(${ROOT_PATH}/third_party/cmake/opencv.cmake)
target_link_opencv(${BINARY})

# install
include(${ROOT_PATH}/cmake/common/install_macro.cmake)
install_target(${BINARY})

# unset
unset(SOURCE)
unset(OBJECT)
unset(BINARY)
unset(TARGET_DIRECTORY)
unset(DEPEND_LIBRARY)
unset(SYSTEM_LIBRARY)
unset(THIRD_PARTY_LIBRARY)

