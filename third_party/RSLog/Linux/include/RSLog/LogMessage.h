//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include "LogLevel.h"
#include "RSLogExport.h"

namespace RSLog {

/**
 * @brief Log message structure containing complete log information
 */
struct RSLOG_API LogMessage {
    std::chrono::system_clock::time_point timestamp; ///< Log timestamp
    std::thread::id threadId;                       ///< Thread ID
    LogLevel level;                                 ///< Log level
    std::string message;                            ///< Log message content
    std::string fileName;                           ///< Source file name
    int lineNumber;                                 ///< Source file line number
    std::string functionName;                       ///< Function name

    /**
     * @brief Constructor
     * @param lvl Log level
     * @param msg Log message
     * @param file Source file name
     * @param line Line number
     * @param function Function name
     */
    LogMessage(LogLevel lvl, const std::string& msg, const std::string& file = "", 
               int line = 0, const std::string& function = "")
        : timestamp(std::chrono::system_clock::now()),
          threadId(std::this_thread::get_id()),
          level(lvl),
          message(msg),
          fileName(file),
          lineNumber(line),
          functionName(function) {}

    /**
     * @brief Format log message as string
     * @param includeMetadata Whether to include metadata (timestamp, thread ID, etc.)
     * @return Formatted string
     */
    std::string toString(bool includeMetadata = true) const {
        std::ostringstream oss;
        
        if (includeMetadata) {
            // Format timestamp
            auto timeT = std::chrono::system_clock::to_time_t(timestamp);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp.time_since_epoch()).count() % 1000;
            
            std::tm localTime;
            #ifdef _WIN32
                localtime_s(&localTime, &timeT);
            #else
                localtime_r(&timeT, &localTime);
            #endif
            
            oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") 
                << '.' << std::setfill('0') << std::setw(3) << ms << " ";
            
            // Thread ID
            oss << "[" << std::hex << std::this_thread::get_id() << "] ";
            
            // Log level
            oss << "[" << LogLevelToString(level) << "] ";
            
            // Source file information
            if (!fileName.empty()) {
                const size_t lastSlash = fileName.find_last_of("/\\");
                const std::string shortFileName = (lastSlash != std::string::npos) ? 
                    fileName.substr(lastSlash + 1) : fileName;
                
                oss << "[" << shortFileName << ":" << lineNumber;
                if (!functionName.empty()) {
                    oss << "(" << functionName << ")";
                }
                oss << "] ";
            }
        }
        
        // Log content
        oss << message;
        
        return oss.str();
    }
};

} // namespace RSLog
