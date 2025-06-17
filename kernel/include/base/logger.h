/**
 * liuxuan 2025.05.27
 *
 */
#ifndef _LOG_H_
#define _LOG_H_

#include "macros.h"

#include <fstream>
#include <mutex>

#pragma warning(push)
#pragma warning(disable : 4251)

class RS_PUBLIC LogFileSink {
    // 同步日志,适用于客户端较中小项目的IO写入{
public:
    static LogFileSink &Instance();

    LogFileSink(const LogFileSink &) =
        delete; // 禁用复制构造和赋值运算符,构造函数和析构函数不通过公开接口公开.

    LogFileSink &operator=(const LogFileSink &) = delete;

    LogFileSink(LogFileSink &&) = delete;
    LogFileSink &operator=(LogFileSink &&) = delete;

    void Init(const std::string &filename = "./rayshape.log", size_t max_size = 100 * 1024 * 1024);

    void DeInit(); // 对内接口

    void Write(const std::string &message); // 对内接口

private:
    void RollFile();

    LogFileSink();

    ~LogFileSink();

private:
    std::ofstream file_;
    std::mutex mutex_;
    std::string filename_;
    size_t max_size_ = 0;
    size_t current_size_ = 0;
};

// RS_LOG
#ifdef __ANDROID__
#include <android/log.h>
#define RS_LOGDT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_DEBUG, tag, ("%s [File %s][Line %d] " fmt),                \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGD] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        fprintf(stdout, "%s", buffer);                                                             \
        LogFileSink::Instance().Write(buffer);                                                     \
    } while (0)

#define RS_LOGIT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_INFO, tag, ("%s [File %s][Line %d] " fmt),                 \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGI] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        fprintf(stdout, "%s", buffer);                                                             \
        LogFileSink::Instance().Write(buffer);                                                     \
    } while (0)

#define RS_LOGET(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        __android_log_print(ANDROID_LOG_ERROR, tag, ("%s [File %s][Line %d] " fmt),                \
                            __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);               \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGI] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                          \
        fprintf(stderr, "%s", buffer);                                                             \
        LogFileSink::Instance().Write(buffer);                                                     \
    } while (0)

#else // win linux macos

#define RS_LOGDT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGD] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        fprintf(stdout, "%s", buffer);                                                             \
        LogFileSink::Instance().Write(buffer);                                                     \
    } while (0)

#define RS_LOGIT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGI] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        fprintf(stdout, "%s", buffer);                                                             \
        LogFileSink::Instance().Write(buffer);                                                     \
    } while (0)

#define RS_LOGET(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGE] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        fprintf(stderr, "%s", buffer);                                                             \
        LogFileSink::Instance().Write(buffer);                                                     \
    } while (0)

#define RS_LOGWT(fmt, tag, ...)                                                                    \
    do {                                                                                           \
        char buffer[4096];                                                                         \
        snprintf(buffer, sizeof(buffer), ("[RS_LOGW] %s: %s [File %s][Line %d] " fmt), tag,        \
                 __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        fprintf(stderr, "%s", buffer);                                                             \
        LogFileSink::Instance().Write(buffer);                                                     \
    } while (0)

#endif //__ANDROID__

#define RS_LOGD(fmt, ...) RS_LOGDT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGI(fmt, ...) RS_LOGIT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGE(fmt, ...) RS_LOGET(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGW(fmt, ...) RS_LOGWT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)

#define RS_LOGE_IF(cond, fmt, ...)                                                                 \
    if (cond) {                                                                                    \
        RS_LOGET(fmt, RS_DEFAULT_STR, ##__VA_ARGS__);                                              \
    }

#include <cassert>
#define ASSERT(x)                                                                                  \
    {                                                                                              \
        int res = (x);                                                                             \
        if (!res) {                                                                                \
            RS_LOGE("Error: assert failed\n");                                                        \
            assert(res);                                                                           \
        }                                                                                          \
    }

#pragma warning(pop)

#endif // _LOG_H_