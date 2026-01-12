/**
 * @file platform_utils.cpp
 * @brief Cross-platform utility functions implementation
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 */

#include "utils/platform_utils.h"

#include <chrono>
#include <thread>
#include <cstdlib>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#define getcwd _getcwd
// Avoid naming conflicts with Windows API
#ifdef GetEnvironmentVariable
#undef GetEnvironmentVariable
#endif
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <climits>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace rayshape
{
    namespace tools
    {
        namespace utils
        {

            std::string PlatformUtils::GetPlatformName() {
#ifdef _WIN32
                return "Windows";
#elif defined(__linux__)
                return "Linux";
#elif defined(__APPLE__)
                return "macOS";
#else
                return "Unknown";
#endif
            }

            char PlatformUtils::GetPathSeparator() {
#ifdef _WIN32
                return '\\';
#else
                return '/';
#endif
            }

            std::string PlatformUtils::ToPlatformPath(const std::string &path) {
                std::string result = path;
                char sep = GetPathSeparator();
                char other_sep = (sep == '\\') ? '/' : '\\';

                for (char &c : result) {
                    if (c == other_sep) {
                        c = sep;
                    }
                }

                return result;
            }

            std::string PlatformUtils::GetEnvironmentVariable(const std::string &var_name,
                                                              const std::string &default_value) {
#ifdef _WIN32
                // Use Windows API for better Unicode support
                char buffer[32767];  // Max environment variable length on Windows
                DWORD result = ::GetEnvironmentVariableA(var_name.c_str(), buffer, sizeof(buffer));
                if (result > 0 && result < sizeof(buffer)) {
                    return std::string(buffer);
                }
                return default_value;
#else
                const char *value = std::getenv(var_name.c_str());
                return value ? std::string(value) : default_value;
#endif
            }

            bool PlatformUtils::IsWindows() {
#ifdef _WIN32
                return true;
#else
                return false;
#endif
            }

            bool PlatformUtils::IsLinux() {
#ifdef __linux__
                return true;
#else
                return false;
#endif
            }

            bool PlatformUtils::IsMacOS() {
#ifdef __APPLE__
                return true;
#else
                return false;
#endif
            }

            std::string PlatformUtils::GetCurrentWorkingDirectory() {
                char buffer[4096];
                if (getcwd(buffer, sizeof(buffer)) != nullptr) {
                    return std::string(buffer);
                }
                return "";
            }

            std::string PlatformUtils::GetExecutableDirectory() {
#ifdef _WIN32
                char buffer[MAX_PATH];
                DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
                if (length > 0) {
                    std::string path(buffer);
                    size_t pos = path.find_last_of("\\/");
                    if (pos != std::string::npos) {
                        return path.substr(0, pos);
                    }
                }
                return "";
#elif defined(__linux__)
                char buffer[PATH_MAX];
                ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
                if (length != -1) {
                    buffer[length] = '\0';
                    std::string path(buffer);
                    size_t pos = path.find_last_of('/');
                    if (pos != std::string::npos) {
                        return path.substr(0, pos);
                    }
                }
                return "";
#elif defined(__APPLE__)
                char buffer[PATH_MAX];
                uint32_t size = sizeof(buffer);
                if (_NSGetExecutablePath(buffer, &size) == 0) {
                    std::string path(buffer);
                    size_t pos = path.find_last_of('/');
                    if (pos != std::string::npos) {
                        return path.substr(0, pos);
                    }
                }
                return "";
#else
                return "";
#endif
            }

            std::string PlatformUtils::GetHomeDirectory() {
#ifdef _WIN32
                char buffer[MAX_PATH];
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, buffer))) {
                    return std::string(buffer);
                }
                // Fallback to environment variables
                std::string home = GetEnvironmentVariable("USERPROFILE");
                if (home.empty()) {
                    std::string drive = GetEnvironmentVariable("HOMEDRIVE");
                    std::string path = GetEnvironmentVariable("HOMEPATH");
                    if (!drive.empty() && !path.empty()) {
                        home = drive + path;
                    }
                }
                return home;
#else
                // Try environment variable first
                std::string home = GetEnvironmentVariable("HOME");
                if (!home.empty()) {
                    return home;
                }

                // Fallback to passwd entry
                struct passwd *pw = getpwuid(getuid());
                if (pw && pw->pw_dir) {
                    return std::string(pw->pw_dir);
                }

                return "";
#endif
            }

            void PlatformUtils::Sleep(int milliseconds) {
                std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            }

            long long PlatformUtils::GetCurrentTimestamp() {
                auto now = std::chrono::system_clock::now();
                auto duration = now.time_since_epoch();
                return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            }

            std::string PlatformUtils::FormatTimestamp(long long timestamp,
                                                       const std::string &format) {
                auto time_point = std::chrono::system_clock::from_time_t(timestamp / 1000);
                auto time_t = std::chrono::system_clock::to_time_t(time_point);

                std::stringstream ss;
                ss << std::put_time(std::localtime(&time_t), format.c_str());
                return ss.str();
            }

        } // namespace utils
    } // namespace tools
} // namespace rayshape