/*
  define 一些常用的宏
*/

#ifndef _MACROS_H_
#define _MACROS_H_

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(__linux__)
#include <stdint.h>
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

#define RS_PI 3.14159265358979323846264338327950288

#ifndef RS_MIN
#define RS_MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef RS_MAX
#define RS_MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#endif //_MACROS_H_