# # 打包宏函数
cmake_minimum_required(VERSION 3.10)

set(CPACK_PACKAGE_DIRECTORY ${ROOT_PATH}/build/package)                      # CPack 的生成路径
# # 许可证文件
#set(CPACK_RESOURCE_FILE_LICENSE "${ROOT_PATH}/LICENSE")         # 版权文件 一般不需要
set(CPACK_PACKAGE_VERSION_MAJOR "${RS_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${RS_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${RS_PATCH_VERSION}")
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.${RS_BUILD_VERSION}) #与最外围的保持一致
## 选择忽略文件或文件夹
# set(CPACK_SOURCE_IGNORE_FILES
#         ${ROOT_PATH}/build
#         ${ROOT_PATH}/cmake-build-debug
#         ${ROOT_PATH}/pack        
#         ${ROOT_PATH}/.git
#         ${ROOT_PATH}/.gitignore
#         ${ROOT_PATH}/.vscode        
# )

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")   # 顶层的项目命名 CMAKE_PROJECT_NAME
set(CPACK_GENERATOR  "TGZ")          # 指定打包的类型是 tar.gz   (RS_Deployment_0.3.0.0.tar.gz) 
# 可以选择的打包类型 7Z(.7z)  7Zzip(.7z) TBZ2(tar.bz2) TGZ(.tar.gz) TXZ(.tar.xz) TZ(.tar.Z) ZIP(.zip)
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION})  

# # 启用 CPack
include(CPack)

# 还需要手动