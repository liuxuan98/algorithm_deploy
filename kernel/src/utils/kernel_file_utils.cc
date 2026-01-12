#include "utils/kernel_file_utils.h"

#include <vector>
#include <queue>
#include <set>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "base/logger.h"

namespace rayshape
{
    namespace utils
    {
// 获取平台相关的路径分隔符
#ifdef _WIN32
        const std::vector<char> PATH_SEPS = {'\\', '/'};
#else
        const std::vector<char> PATH_SEPS = {'/'};
#endif

        bool DirectoryExists(const std::string &directory) {
#ifdef _WIN32
            DWORD attributes = GetFileAttributesA(directory.c_str());
            return (attributes != INVALID_FILE_ATTRIBUTES
                    && (attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
            struct stat info;
            if (stat(directory.c_str(), &info) != 0) {
                return false; // 路径不存在或无法访问
            }
            return S_ISDIR(info.st_mode); // 判断是否为目录
#endif
        }

        ErrorCode CreateDir(const std::string &directory, bool exist_ok) {
            std::set<size_t> sep_pos; // pos 按升序排序
            for (const auto &sep : PATH_SEPS) {
                size_t pos = directory.find_first_of(sep, 0);
                sep_pos.insert(pos);
                while (pos != std::string::npos) {
                    pos = directory.find_first_of(sep, pos + 1);
                    sep_pos.insert(pos);
                }
            }

            for (const auto &pos : sep_pos) {
                // RS_LOGD("pos: %zu\n", pos);
                std::string cur_dir = directory.substr(0, pos);
                // RS_LOGD("cur_dir: %s\n", cur_dir.c_str());
                if (DirectoryExists(cur_dir) && exist_ok) {
                    continue;
                }
                int ret = 0;
#ifdef _WIN32
                ret = _mkdir(cur_dir.c_str());
#else
                ret = mkdir(cur_dir.c_str(), 0777);
#endif
                if (ret != 0) {
                    return ErrorCode::RS_UNKNOWN;
                }
            }
            return ErrorCode::RS_SUCCESS;
        }

        std::string FindFileInDirectory(const std::string &directory, const std::string &fname,
                                        bool recursive) {
            if (!DirectoryExists(directory)) {
                return "";
            }

            std::queue<std::string> dirs;
            dirs.push(directory);

            while (!dirs.empty()) {
                std::string current_dir = dirs.front();
                dirs.pop();

// 打开目录
#ifdef _WIN32
                // TODO: 未测试
                WIN32_FIND_DATA find_data;
                std::string search_path = current_dir + "\\*";
                HANDLE h_find = FindFirstFile(search_path.c_str(), &find_data);
                if (h_find == INVALID_HANDLE_VALUE)
                    continue;

                do {
                    std::string name = find_data.cFileName;

                    // 忽略 . 和 ..
                    if (name == "." || name == "..")
                        continue;

                    std::string full_path = current_dir + "\\" + name;

                    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        if (recursive) {
                            dirs.push(full_path);
                        }
                    } else {
                        if (name == fname) {
                            FindClose(h_find);
                            return full_path;
                        }
                    }
                } while (FindNextFile(h_find, &find_data));
                FindClose(h_find);
#else
                DIR *dir = opendir(current_dir.c_str());
                if (!dir)
                    continue;

                struct dirent *entry;
                while ((entry = readdir(dir)) != nullptr) {
                    std::string name = entry->d_name;

                    // 忽略 . 和 ..
                    if (name == "." || name == "..")
                        continue;

                    std::string full_path = current_dir + "/" + name;

                    struct stat statbuf;
                    if (stat(full_path.c_str(), &statbuf) == -1)
                        continue;

                    if (S_ISDIR(statbuf.st_mode)) {
                        if (recursive) {
                            dirs.push(full_path);
                        }
                    } else {
                        if (name == fname) {
                            closedir(dir);
                            return full_path;
                        }
                    }
                }
                closedir(dir);
#endif
            }
            return ""; // 没有找到文件
        }

        std::vector<std::string> GetFilesInDirectory(const std::string &directory, bool recursive) {
            std::vector<std::string> files;

#ifdef _WIN32
            // Windows 实现
            WIN32_FIND_DATA findData;
            std::string searchPath = directory + "\\*";
            HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);

            if (hFind == INVALID_HANDLE_VALUE) {
                return files;
            }

            do {
                std::string fileName = findData.cFileName;
                if (fileName == "." || fileName == "..") {
                    continue;
                }

                std::string fullPath = directory + "\\" + fileName;

                if (recursive && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    // 递归处理子目录
                    std::vector<std::string> subFiles = GetFilesInDirectory(fullPath, recursive);
                    files.insert(files.end(), subFiles.begin(), subFiles.end());
                } else if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    // 添加文件路径
                    files.push_back(fullPath);
                }

            } while (FindNextFile(hFind, &findData));

            FindClose(hFind);

#else
            // POSIX 实现（Linux/macOS）
            DIR *dir = opendir(directory.c_str());
            if (!dir) {
                return files;
            }

            struct dirent *entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string fileName = entry->d_name;
                if (fileName == "." || fileName == "..") {
                    continue;
                }

                std::string fullPath = directory + "/" + fileName;

                struct stat statbuf;
                if (stat(fullPath.c_str(), &statbuf) == -1) {
                    continue;
                }

                if (recursive && S_ISDIR(statbuf.st_mode)) {
                    // 递归处理子目录
                    std::vector<std::string> subFiles = GetFilesInDirectory(fullPath, recursive);
                    files.insert(files.end(), subFiles.begin(), subFiles.end());
                } else if (S_ISREG(statbuf.st_mode)) {
                    // 添加文件路径
                    files.push_back(fullPath);
                }
            }

            closedir(dir);
#endif

            return files;
        }

        std::string JoinPath(const std::string &prefix, const std::string &name) {
            std::string result;

            // 如果 prefix 为空，则直接返回 name
            if (prefix.empty()) {
                return name;
            }

            // 如果 prefix 已经以分隔符结尾，直接拼接 name
            char sep = 0;
            for (int i = 0; i < PATH_SEPS.size(); i++) {
                if (prefix.back() == PATH_SEPS.at(i)) {
                    sep = PATH_SEPS.at(i);
                }
            }
            if (sep) {
                result = prefix + name;
            } else {
                result = prefix + PATH_SEPS.at(0) + name;
            }

            return result;
        }

        std::vector<std::string> SplitPath(const std::string &path) {
            std::string prefix, fname, extension;

            size_t pos = 0;
            for (const auto &sep : PATH_SEPS) {
                size_t cur_pos = path.find_last_of(sep);
                if (cur_pos != std::string::npos && cur_pos > pos) {
                    pos = cur_pos;
                }
            }

            if (pos > 0) {
                prefix = path.substr(0, pos + 1);
                fname = path.substr(pos + 1);
            } else {
                fname = path;
            }
            if (fname != "." && fname != "..") {
                pos = fname.find_first_of('.');
                extension = fname.substr(pos);
                fname = fname.substr(0, pos);
            }

            return {prefix, fname, extension};
        }
    } // namespace utils
} // namespace rayshape