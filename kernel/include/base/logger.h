/**
 * liuxuan 2025.05.27
 * Updated to use RSLog - 2025.01.XX
 */
#ifndef LOGGER_H
#define LOGGER_H

#include "macros.h"

#include <string>
#include <cassert>

#ifdef __cplusplus
#include <array>
#include <sstream>
#include <type_traits>
#endif

// Constants to replace magic numbers
namespace RSLogConstants
{
    static constexpr size_t DEFAULT_LOG_FILE_SIZE_MB = 100;
    static constexpr size_t KB_SIZE = 1024;
    static constexpr size_t DEFAULT_MAX_LOG_SIZE = DEFAULT_LOG_FILE_SIZE_MB * KB_SIZE * KB_SIZE;
    static constexpr size_t LOG_BUFFER_SIZE = 4096;
} // namespace RSLogConstants

// Forward declare RSLog functionality
namespace RSLog
{
    class Logger;
    enum class LogLevel;
} // namespace RSLog

// Initialize RSLog system
extern RS_PUBLIC void InitRSLogSystem();

// Shutdown RSLog system
extern RS_PUBLIC void ShutdownRSLogSystem();

// RSLog wrapper functions
extern RS_PUBLIC void RSLogDebug(const std::string &message);
extern RS_PUBLIC void RSLogInfo(const std::string &message);
extern RS_PUBLIC void RSLogWarn(const std::string &message);
extern RS_PUBLIC void RSLogError(const std::string &message);

// Legacy LogFileSink class for backward compatibility
class RS_PUBLIC LogFileSink {
public:
    static LogFileSink &Instance();

    LogFileSink(const LogFileSink &) = delete;
    LogFileSink &operator=(const LogFileSink &) = delete;
    LogFileSink(LogFileSink &&) = delete;
    LogFileSink &operator=(LogFileSink &&) = delete;

    void Init(const std::string &filename = "./rayshape.log",
              size_t max_size = RSLogConstants::DEFAULT_MAX_LOG_SIZE);
    void DeInit();
    void Write(const std::string &message);

private:
    LogFileSink();
    ~LogFileSink();
    void RollFile();

private:
    std::string filename_;
    size_t max_size_ = 0;
    size_t current_size_ = 0;
    bool initialized_ = false;
};

// RS_LOG macros using RSLog
#ifdef __ANDROID__
#include <android/log.h>

#ifdef __cplusplus
#define RS_LOGDT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_DEBUG, tag, ("%s [File %s][Line %d] " fmt),                \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        std::array<char, RSLogConstants::LOG_BUFFER_SIZE> buffer{};                                \
        snprintf(buffer.data(), buffer.size(), ("[RS_LOGD] %s: %s [File %s][Line %d] " fmt), tag,  \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        RSLogDebug(std::string(buffer.data()));                                                    \
    } while (0)

#define RS_LOGIT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_INFO, tag, ("%s [File %s][Line %d] " fmt),                 \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        std::array<char, RSLogConstants::LOG_BUFFER_SIZE> buffer{};                                \
        snprintf(buffer.data(), buffer.size(), ("[RS_LOGI] %s: %s [File %s][Line %d] " fmt), tag,  \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        RSLogInfo(std::string(buffer.data()));                                                     \
    } while (0)

#define RS_LOGET(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_ERROR, tag, ("%s [File %s][Line %d] " fmt),                \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        std::array<char, RSLogConstants::LOG_BUFFER_SIZE> buffer{};                                \
        snprintf(buffer.data(), buffer.size(), ("[RS_LOGE] %s: %s [File %s][Line %d] " fmt), tag,  \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        RSLogError(std::string(buffer.data()));                                                    \
    } while (0)
#else
// C version for Android
#define RS_LOGDT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_DEBUG, tag, ("%s [File %s][Line %d] " fmt),                \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGD] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        RSLogDebug(buffer);                                                                        \
    } while (0)

#define RS_LOGIT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_INFO, tag, ("%s [File %s][Line %d] " fmt),                 \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGI] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        RSLogInfo(buffer);                                                                         \
    } while (0)

#define RS_LOGET(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_ERROR, tag, ("%s [File %s][Line %d] " fmt),                \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGE] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        RSLogError(buffer);                                                                        \
    } while (0)
#endif // __cplusplus

#else // win linux macos

#ifdef __cplusplus
#define RS_LOGDT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        std::array<char, RSLogConstants::LOG_BUFFER_SIZE> buffer{};                                \
        snprintf(buffer.data(), buffer.size(), ("[RS_LOGD] %s: %s [File %s][Line %d] " fmt), tag,  \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        RSLogDebug(std::string(buffer.data()));                                                    \
    } while (0)

#define RS_LOGIT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        std::array<char, RSLogConstants::LOG_BUFFER_SIZE> buffer{};                                \
        snprintf(buffer.data(), buffer.size(), ("[RS_LOGI] %s: %s [File %s][Line %d] " fmt), tag,  \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        RSLogInfo(std::string(buffer.data()));                                                     \
    } while (0)

#define RS_LOGET(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        std::array<char, RSLogConstants::LOG_BUFFER_SIZE> buffer{};                                \
        snprintf(buffer.data(), buffer.size(), ("[RS_LOGE] %s: %s [File %s][Line %d] " fmt), tag,  \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        RSLogError(std::string(buffer.data()));                                                    \
    } while (0)

#define RS_LOGWT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        std::array<char, RSLogConstants::LOG_BUFFER_SIZE> buffer{};                                \
        snprintf(buffer.data(), buffer.size(), ("[RS_LOGW] %s: %s [File %s][Line %d] " fmt), tag,  \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        RSLogWarn(std::string(buffer.data()));                                                     \
    } while (0)
#else
// C version for other platforms
#define RS_LOGDT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGD] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        RSLogDebug(buffer);                                                                        \
    } while (0)

#define RS_LOGIT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGI] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        RSLogInfo(buffer);                                                                         \
    } while (0)

#define RS_LOGET(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGE] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        RSLogError(buffer);                                                                        \
    } while (0)

#define RS_LOGWT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGW] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        RSLogWarn(buffer);                                                                         \
    } while (0)
#endif // __cplusplus

#endif //__ANDROID__

// Modern C++ alternatives to variadic macros
#ifdef __cplusplus
namespace RSLogDetail
{
    // Helper implementation for case with no arguments
    inline std::string FormatLogMessageImpl(const std::string &level, const std::string &tag,
                                            const std::string &function, const std::string &file,
                                            int line, const std::string &fmt, std::true_type) {
        std::ostringstream oss;
        oss << "[" << level << "] " << tag << ": " << function << " [File " << file << "][Line "
            << line << "] " << fmt;
        return oss.str();
    }

    // Helper implementation for case with arguments
    template <typename... Args>
    inline std::string FormatLogMessageImpl(const std::string &level, const std::string &tag,
                                            const std::string &function, const std::string &file,
                                            int line, const std::string &fmt, std::false_type,
                                            Args &&...args) {
        std::ostringstream oss;
        oss << "[" << level << "] " << tag << ": " << function << " [File " << file << "][Line "
            << line << "] ";

        std::array<char, RSLogConstants::LOG_BUFFER_SIZE> buffer{};
        snprintf(buffer.data(), buffer.size(), fmt.c_str(), args...);
        oss << std::string(buffer.data());
        return oss.str();
    }

    // Helper function to format log messages
    template <typename... Args>
    inline std::string FormatLogMessage(const std::string &level, const std::string &tag,
                                        const std::string &function, const std::string &file,
                                        int line, const std::string &fmt, Args &&...args) {
        // Use tag dispatch based on whether we have arguments
        return FormatLogMessageImpl(level, tag, function, file, line, fmt,
                                    std::integral_constant<bool, sizeof...(args) == 0>{},
                                    std::forward<Args>(args)...);
    }

    // Modern C++ logging functions that avoid macros
    template <typename... Args>
    inline void LogDebug(const std::string &function, const std::string &file, int line,
                         const std::string &fmt, Args &&...args) {
        std::string message = FormatLogMessage("RS_LOGD", RS_DEFAULT_STR, function, file, line, fmt,
                                               std::forward<Args>(args)...);
        RSLogDebug(message);
    }

    template <typename... Args>
    inline void LogInfo(const std::string &function, const std::string &file, int line,
                        const std::string &fmt, Args &&...args) {
        std::string message = FormatLogMessage("RS_LOGI", RS_DEFAULT_STR, function, file, line, fmt,
                                               std::forward<Args>(args)...);
        RSLogInfo(message);
    }

    template <typename... Args>
    inline void LogError(const std::string &function, const std::string &file, int line,
                         const std::string &fmt, Args &&...args) {
        std::string message = FormatLogMessage("RS_LOGE", RS_DEFAULT_STR, function, file, line, fmt,
                                               std::forward<Args>(args)...);
        RSLogError(message);
    }

    template <typename... Args>
    inline void LogWarn(const std::string &function, const std::string &file, int line,
                        const std::string &fmt, Args &&...args) {
        std::string message = FormatLogMessage("RS_LOGW", RS_DEFAULT_STR, function, file, line, fmt,
                                               std::forward<Args>(args)...);
        RSLogWarn(message);
    }

    template <typename Condition, typename... Args>
    inline void LogErrorIf(Condition &&condition, const std::string &function,
                           const std::string &file, int line, const std::string &fmt,
                           Args &&...args) {
        if (condition) {
            LogError(function, file, line, fmt, std::forward<Args>(args)...);
        }
    }
} // namespace RSLogDetail

// Modern C++ logging interface - preferred for new C++ code
#define RS_LOGD_CPP(fmt, ...)                                                                      \
    RSLogDetail::LogDebug(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RS_LOGI_CPP(fmt, ...)                                                                      \
    RSLogDetail::LogInfo(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RS_LOGE_CPP(fmt, ...)                                                                      \
    RSLogDetail::LogError(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RS_LOGW_CPP(fmt, ...)                                                                      \
    RSLogDetail::LogWarn(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RS_LOGE_IF_CPP(cond, fmt, ...)                                                             \
    RSLogDetail::LogErrorIf(cond, __FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// Legacy macro interface for compatibility - use *_CPP versions for new code
#ifndef RS_LOG_USE_LEGACY_MACROS
// Use modern C++ interface by default
#define RS_LOGD(fmt, ...) RS_LOGD_CPP(fmt, ##__VA_ARGS__)
#define RS_LOGI(fmt, ...) RS_LOGI_CPP(fmt, ##__VA_ARGS__)
#define RS_LOGE(fmt, ...) RS_LOGE_CPP(fmt, ##__VA_ARGS__)
#define RS_LOGW(fmt, ...) RS_LOGW_CPP(fmt, ##__VA_ARGS__)
#define RS_LOGE_IF(cond, fmt, ...) RS_LOGE_IF_CPP(cond, fmt, ##__VA_ARGS__)
#else
// Use legacy implementation when explicitly requested
#define RS_LOGD_IMPL(func, file, line, fmt, ...)                                                   \
    do {                                                                                           \
        std::string message = RSLogDetail::FormatLogMessage("RS_LOGD", RS_DEFAULT_STR, func, file, \
                                                            line, fmt, ##__VA_ARGS__);             \
        RSLogDebug(message);                                                                       \
    } while (0)

#define RS_LOGI_IMPL(func, file, line, fmt, ...)                                                   \
    do {                                                                                           \
        std::string message = RSLogDetail::FormatLogMessage("RS_LOGI", RS_DEFAULT_STR, func, file, \
                                                            line, fmt, ##__VA_ARGS__);             \
        RSLogInfo(message);                                                                        \
    } while (0)

#define RS_LOGE_IMPL(func, file, line, fmt, ...)                                                   \
    do {                                                                                           \
        std::string message = RSLogDetail::FormatLogMessage("RS_LOGE", RS_DEFAULT_STR, func, file, \
                                                            line, fmt, ##__VA_ARGS__);             \
        RSLogError(message);                                                                       \
    } while (0)

#define RS_LOGW_IMPL(func, file, line, fmt, ...)                                                   \
    do {                                                                                           \
        std::string message = RSLogDetail::FormatLogMessage("RS_LOGW", RS_DEFAULT_STR, func, file, \
                                                            line, fmt, ##__VA_ARGS__);             \
        RSLogWarn(message);                                                                        \
    } while (0)

// User-facing macros that capture correct caller info
#define RS_LOGD(fmt, ...) RS_LOGD_IMPL(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RS_LOGI(fmt, ...) RS_LOGI_IMPL(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RS_LOGE(fmt, ...) RS_LOGE_IMPL(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RS_LOGW(fmt, ...) RS_LOGW_IMPL(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define RS_LOGE_IF(cond, fmt, ...)                                                                 \
    do {                                                                                           \
        if (cond) {                                                                                \
            RS_LOGE(fmt, ##__VA_ARGS__);                                                           \
        }                                                                                          \
    } while (0)
#endif // RS_LOG_USE_LEGACY_MACROS

#else
// For C code, keep the original macro definitions
#define RS_LOGD(fmt, ...) RS_LOGDT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGI(fmt, ...) RS_LOGIT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGE(fmt, ...) RS_LOGET(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGW(fmt, ...) RS_LOGWT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)

#define RS_LOGE_IF(cond, fmt, ...)                                                                 \
    if (cond) {                                                                                    \
        RS_LOGET(fmt, RS_DEFAULT_STR, ##__VA_ARGS__);                                              \
    }
#endif // __cplusplus

// Replace function-like macro with constexpr template function
#ifdef __cplusplus
template <typename T> inline void ASSERT(T &&condition) {
    if (!condition) {
        RS_LOGE("Error: assert failed\n");
        assert(static_cast<bool>(condition));
    }
}
#else
// For C code, keep the macro version
#define ASSERT(x)                                                                                  \
    {                                                                                              \
        int res = (x);                                                                             \
        if (!res) {                                                                                \
            RS_LOGE("Error: assert failed\n");                                                     \
            assert(res);                                                                           \
        }                                                                                          \
    }
#endif // __cplusplus

#endif // LOGGER_H
