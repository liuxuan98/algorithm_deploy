#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#endif

// get all files in directory
std::vector<std::string> getFilesInDirectory(const std::string &directoryPath) {
    std::vector<std::string> filenames;

#ifdef _WIN32
    // Windows 平台实现
    WIN32_FIND_DATA fileData;
    HANDLE hFind = FindFirstFile((directoryPath + "\\*").c_str(), &fileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // 排除 "." 和 ".."
            if (std::string(fileData.cFileName) != "." && std::string(fileData.cFileName) != "..") {
                filenames.push_back(fileData.cFileName);
            }
        } while (FindNextFile(hFind, &fileData) != 0);
        FindClose(hFind);
    }
#else
    // POSIX 平台实现（Linux 和 macOS）
    DIR *dir = opendir(directoryPath.c_str());
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            // 排除 "." 和 ".."
            if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
                filenames.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }
#endif

    return filenames;
}
