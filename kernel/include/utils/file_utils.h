#include "base/logger.h"

#include <string>
#include <algorithm>
#include <cstdlib>    // for getenv
#include <stdexcept>  // for runtime_error
#include <sys/stat.h> // 用于mkdir函数
#include <errno.h>    // 用于错误号errno

#ifdef _WIN32
#include <direct.h> // Windows下_mkdir的头文件
#define MKDIR(path) _mkdir(path)
#else
#define MKDIR(path) mkdir(path, 0755) // Unix-like系统使用mkdir并设置权限
#endif

namespace rayshape
{

    namespace utils
    {

        static std::string GetFileParentPath(const std::string &full_name,
                                             bool absolute_path = false) {
            if (full_name.empty()) {
                return "";
            }

            // find last pos of gap symbol.
#ifdef _WIN32
            // Windows 平台支持 '\' 和 '/' 作为路径分隔符
            size_t pos = full_name.find_last_of("\\/");
#else
            // POSIX 系统仅支持 '/' 作为路径分隔符
            size_t pos = full_name.find_last_of('/');
#endif

            // 如果路径中没有分隔符，说明输入的是文件名而不是路径
            if (pos == std::string::npos) {
                RS_LOGE("Invalid path:%s Parent directory not found.\n", full_name.c_str());
                return "";
            }

            // track parent folder.
            std::string parent_path = full_name.substr(0, pos);

            // 如果需要返回绝对路径
            if (absolute_path) {
                char buffer[4096]; // 缓冲区存储绝对路径
#ifdef _WIN32
                // Windows 使用 _fullpath 获取绝对路径
                if (_fullpath(buffer, parent_path.c_str(), sizeof(buffer)) == nullptr) {
                    throw std::runtime_error("Failed to resolve absolute path.");
                }
#else
                // POSIX 使用 realpath 获取绝对路径
                if (realpath(parent_path.c_str(), buffer) == nullptr) {
                    throw std::runtime_error("Failed to resolve absolute path.");
                }
#endif
                parent_path = buffer; // renew
            }

            return parent_path;
            // // 返回最后一个分隔符之前的部分
            // return path.substr(0, last_slash);
        }

        static bool MakeDirectory(const std::string &path) {
            if (path.empty()) {
                return false;
            }

            // 尝试直接创建目录
            if (MKDIR(path.c_str()) == 0) {
                return true;
            }

            // 如果目录已存在，也返回成功
            if (errno == EEXIST) {
                return true;
            }

            // 如果失败是因为父目录不存在，则我们需要递归创建
            if (errno == ENOENT) {
                // 找到路径中最后一个分隔符
                size_t pos = path.find_last_of("/\\");
                if (pos == std::string::npos) {
                    // 没有找到分隔符，无法创建
                    return false;
                }

                // 递归创建父目录
                std::string parent_dir = path.substr(0, pos);
                if (!MakeDirectory(parent_dir)) {
                    return false;
                }

                // 父目录创建成功后，再次尝试创建当前目录
                return MKDIR(path.c_str()) == 0 || errno == EEXIST;
            }

            // 其他错误
            return false;
        }

    } // namespace utils
} // namespace rayshape