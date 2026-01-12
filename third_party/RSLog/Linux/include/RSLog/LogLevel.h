//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include <string>
#include <iostream>
#include "RSLogExport.h"

namespace RSLog {

/**
 * @brief Log level enumeration class
 */
enum class LogLevel {
    DEBUG,   ///< Debug information, detailed system operation information
    INFO,    ///< General information, indicates normal program operation
    WARN,    ///< Warning information, indicates potential problems
    ERROR,   ///< Error information, indicates errors occurred but program can continue
    FATAL    ///< Fatal error, indicates program cannot continue running
};

/**
 * @brief Convert log level to string
 * @param level Log level
 * @return String representation of log level
 */
RSLOG_API inline std::string LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default:              return "UNKNOWN";
    }
}

/**
 * @brief Convert string to log level
 * @param levelStr String representation of log level
 * @return Log level enumeration value
 */
RSLOG_API inline LogLevel StringToLogLevel(const std::string& levelStr) {
    if (levelStr == "DEBUG") return LogLevel::DEBUG;
    if (levelStr == "INFO")  return LogLevel::INFO;
    if (levelStr == "WARN")  return LogLevel::WARN;
    if (levelStr == "ERROR") return LogLevel::ERROR;
    if (levelStr == "FATAL") return LogLevel::FATAL;
    return LogLevel::INFO; // Default to INFO level
}

} // namespace RSLog
