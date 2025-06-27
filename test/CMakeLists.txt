## rayshape_test.exe lauches the test program

set(TEST_PROJECT_NAME rayshape_test)
set(TEST_SOURCE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_LINK_LIBRARY)
set(TEST_SOURCE) 
set(TEST_WORK_FLODER test)

# include
include_directories(${ROOT_PATH}/kernel/include) ## kernel party include

#include gtest
include(${TEST_SOURCE_ROOT_PATH}/../third_party/cmake/googletest.cmake)
set_gtest_lib("")
list(APPEND TEST_LINK_LIBRARY ${gtest_lib})

list(APPEND TEST_LINK_LIBRARY ${LIB_NAME})

#set source file
file(GLOB TEST_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main_test.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_test/test.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel_test/*.cc
)

if(SYSTEM.Windows)
    add_definitions(-DUSING_RS_DLL)
    remove_definitions(-DBUILDING_RS_DLL) #防止sample demo的导出
endif()

# # add binary(exe)
add_executable(${TEST_PROJECT_NAME} ${TEST_SRC_FILES})

# # link lib
target_link_libraries(${TEST_PROJECT_NAME} ${TEST_LINK_LIBRARY})

# set project work folder
set_property(TARGET ${TEST_PROJECT_NAME} PROPERTY FOLDER ${TEST_WORK_FLODER})
#unset 
unset(TEST_PROJECT_NAME)
unset(TEST_SOURCE_ROOT_PATH)
unset(TEST_LINK_LIBRARY)
unset(TEST_SOURCE)
unset(TEST_WORK_FLODER)