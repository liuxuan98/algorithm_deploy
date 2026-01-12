//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

// 为Windows平台定义导出/导入宏
#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(RSLOGKIT_STATIC_LIB)
        // 静态库不需要导出符号
        #define RSLOG_API
    #else
        #if defined(RSLOGKIT_EXPORTS)
            // 导出DLL符号
            #if defined(_MSC_VER)
                #define RSLOG_API __declspec(dllexport)
            #else
                #define RSLOG_API __attribute__((dllexport))
            #endif
        #else
            // 导入DLL符号
            #if defined(_MSC_VER)
                #define RSLOG_API __declspec(dllimport)
            #else
                #define RSLOG_API __attribute__((dllimport))
            #endif
        #endif
    #endif
#else
    // 在非Windows平台上，导出宏为空
    #define RSLOG_API
#endif 