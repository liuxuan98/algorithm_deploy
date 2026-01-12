/**
 * @file platform_utils.h
 * @brief Cross-platform utility functions
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 */

#ifndef PACK_MODELS_UTILS_PLATFORM_UTILS_H_
#define PACK_MODELS_UTILS_PLATFORM_UTILS_H_

#include <string>

namespace rayshape
{
    namespace tools
    {
        namespace utils
        {

            /**
             * @brief Platform utility functions
             */
            class PlatformUtils {
            public:
                /**
                 * @brief Get platform name
                 * @return Platform name string
                 */
                static std::string GetPlatformName();

                /**
                 * @brief Get path separator for current platform
                 * @return Path separator character
                 */
                static char GetPathSeparator();

                /**
                 * @brief Convert path to platform-specific format
                 * @param path Input path
                 * @return Platform-specific path
                 */
                static std::string ToPlatformPath(const std::string &path);

                /**
                 * @brief Get environment variable value
                 * @param var_name Variable name
                 * @param default_value Default value if not found
                 * @return Variable value or default
                 */
                static std::string GetEnvironmentVariable(const std::string &var_name,
                                                          const std::string &default_value = "");

                /**
                 * @brief Check if running on Windows
                 * @return true if Windows, false otherwise
                 */
                static bool IsWindows();

                /**
                 * @brief Check if running on Linux
                 * @return true if Linux, false otherwise
                 */
                static bool IsLinux();

                /**
                 * @brief Check if running on macOS
                 * @return true if macOS, false otherwise
                 */
                static bool IsMacOS();

                /**
                 * @brief Get current working directory
                 * @return Current working directory path
                 */
                static std::string GetCurrentWorkingDirectory();

                /**
                 * @brief Get executable directory
                 * @return Directory containing current executable
                 */
                static std::string GetExecutableDirectory();

                /**
                 * @brief Get user home directory
                 * @return User home directory path
                 */
                static std::string GetHomeDirectory();

                /**
                 * @brief Sleep for specified milliseconds
                 * @param milliseconds Sleep duration in milliseconds
                 */
                static void Sleep(int milliseconds);

                /**
                 * @brief Get current timestamp in milliseconds
                 * @return Timestamp in milliseconds since epoch
                 */
                static long long GetCurrentTimestamp();

                /**
                 * @brief Format timestamp to string
                 * @param timestamp Timestamp in milliseconds
                 * @param format Format string (default: "%Y-%m-%d %H:%M:%S")
                 * @return Formatted timestamp string
                 */
                static std::string FormatTimestamp(long long timestamp,
                                                   const std::string &format = "%Y-%m-%d %H:%M:%S");

            private:
                PlatformUtils() = delete; // Static class
            };

        } // namespace utils
    } // namespace tools
} // namespace rayshape

#endif // PACK_MODELS_UTILS_PLATFORM_UTILS_H_