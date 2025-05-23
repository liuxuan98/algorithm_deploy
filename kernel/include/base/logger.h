
/**
 * @TODO：要不要去使用一个第三方的日志库，比如sdplog
 *
 */
#ifndef _LOG_H_
#define _LOG_H_

#include "macros.h"

// RS_LOG
#ifdef __ANDROID__
#include <android/log.h>
#define RS_LOGDT(fmt, tag, ...)                                                \
  __android_log_print(ANDROID_LOG_DEBUG, tag, ("%s [File %s][Line %d] " fmt),  \
                      __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
  fprintf(stdout, ("D/%s: %s [File %s][Line %d] " fmt), tag,                   \
          __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#define RS_LOGIT(fmt, tag, ...)                                                \
  __android_log_print(ANDROID_LOG_INFO, tag, ("%s [File %s][Line %d] " fmt),   \
                      __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
  fprintf(stdout, ("I/%s: %s [File %s][Line %d] " fmt), tag,                   \
          __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#define RS_LOGET(fmt, tag, ...)                                                \
  __android_log_print(ANDROID_LOG_ERROR, tag, ("%s [File %s][Line %d] " fmt),  \
                      __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
  fprintf(stderr, ("E/%s: %s [File %s][Line %d] " fmt), tag,                   \
          __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define RS_LOGDT(fmt, tag, ...)                                            \
  fprintf(stdout, ("D/%s: %s [File %s][Line %d] " fmt), tag, __FUNCTION__, \
          __FILE__, __LINE__, ##__VA_ARGS__)
#define RS_LOGIT(fmt, tag, ...)                                            \
  fprintf(stdout, ("I/%s: %s [File %s][Line %d] " fmt), tag, __FUNCTION__, \
          __FILE__, __LINE__, ##__VA_ARGS__)
#define RS_LOGET(fmt, tag, ...)                                            \
  fprintf(stderr, ("E/%s: %s [File %s][Line %d] " fmt), tag, __FUNCTION__, \
          __FILE__, __LINE__, ##__VA_ARGS__)
#define RS_LOGWT(fmt, tag, ...)                                            \
  fprintf(stderr, ("W/%s: %s [File %s][Line %d] " fmt), tag, __FUNCTION__, \
          __FILE__, __LINE__, ##__VA_ARGS__)
#endif //__ANDROID__

#define RS_LOGD(fmt, ...) \
  RS_LOGDT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGI(fmt, ...) \
  RS_LOGIT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGE(fmt, ...) \
  RS_LOGET(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)
#define RS_LOGW(fmt, ...) \
  RS_LOGWT(fmt, RS_DEFAULT_STR, ##__VA_ARGS__)

#define RS_LOGE_IF(cond, fmt, ...)                \
  if (cond)                                       \
  {                                               \
    RS_LOGET(fmt, RS_DEFAULT_STR, ##__VA_ARGS__); \
  }

#endif // _LOG_H_