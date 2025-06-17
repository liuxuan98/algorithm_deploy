# set
set(SOURCE)
set(OBJECT)
set(BINARY image_classifier_sample)
set(TARGET_DIRECTORY sample)
set(DEPEND_LIBRARY)
set(SYSTEM_LIBRARY)
set(THIRD_PARTY_LIBRARY)

# include
include_directories(${ROOT_PATH}/kernel/include)

# source
file(GLOB SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

# lib dll import
if(SYSTEM.Windows)
    add_definitions(-DUSING_RS_DLL)
    remove_definitions(-DBUILDING_RS_DLL) #防止sample demo的导出
endif()
# object

# binary
add_executable(${BINARY} ${SOURCE} ${OBJECT})

# target_directory
set_property(TARGET ${BINARY} PROPERTY FOLDER ${TARGET_DIRECTORY})

# depend_library rs_core
#message(STATUS "LIB_NAME = ${LIB_NAME}")
#list(APPEND DEPEND_LIBRARY ${LIB_NAME})

target_link_libraries(${BINARY} PRIVATE ${LIB_NAME})

# system_library

# third_party_library
## link opencv
include(${ROOT_PATH}/third_party/cmake/opencv.cmake)
set_opencv_lib("world")

list(APPEND THIRD_PARTY_LIBRARY ${opencv_lib})
target_link_libraries(${BINARY} PRIVATE ${THIRD_PARTY_LIBRARY})

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






