/*
  define 一些常用的宏
*/

#ifndef MACROS_H
#define MACROS_H

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#if defined(_WIN32) || defined(__linux__)
#include <cstdint>
#endif

/*
 *
 * @brief interface
 *
 */
#if defined(_MSC_VER)
#if defined(BUILDING_RS_DLL)
#define RS_PUBLIC __declspec(dllexport)
#elif defined(USING_RS_DLL)
#define RS_PUBLIC __declspec(dllimport)
#else
#define RS_PUBLIC
#endif // BUILDING_RS_DLL
#else  // _MSC_VER/WINDOWS
#define RS_PUBLIC __attribute__((visibility("default")))
#endif // _MSC_VER

#ifdef __cplusplus
#define RS_C_API extern "C" RS_PUBLIC
#else
#define RS_C_API RS_PUBLIC
#endif

/**
 * @brief math
 *
 */

static constexpr double RS_PI = 3.14159265358979323846264338327950288;

#ifdef __cplusplus
// Replace function-like macros with constexpr template functions
template <typename T> constexpr const T &RS_MIN(const T &x, const T &y) noexcept {
    return (x < y) ? x : y;
}

template <typename T> constexpr const T &RS_MAX(const T &x, const T &y) noexcept {
    return (x > y) ? x : y;
}
#else
// For C code, keep the macro versions
#ifndef RS_MIN
#define RS_MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef RS_MAX
#define RS_MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#endif // __cplusplus

/**
 * @brief string
 *
 */
#define RS_DEFAULT_STR ""

// 性能优化 gcc内置函数 apple and unix平台上可用
#ifdef _ENABLE_LIKELY_
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely
#define unlikely
#endif

#endif //_MACROS_H_//#endif // MACROS_H
