//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <functional>
#include <cstring>

#include "LogLevel.h"
#include "LogMessage.h"
#include "LogBackend.h"
#include "LogQueue.h"
#include "LogFormatter.h"
#include "RSLogExport.h"

namespace RSLog {

/**
 * @brief Version information
 */
constexpr const char* VERSION = "1.4.1";
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 4;
constexpr int VERSION_PATCH = 1;

/**
 * @brief Logger configuration
 */
struct RSLOG_API LoggerConfig {
    std::string name;                    ///< Logger name
    LogLevel level;                      ///< Default log level
    bool asyncMode;                      ///< Whether to use async mode
    size_t asyncQueueSize;               ///< Async queue size
    std::vector<std::string> backends;   ///< List of backends to use
    bool autoFlush;                      ///< Whether to enable automatic flush
    int flushIntervalSeconds;            ///< Auto flush interval in seconds (0 = disabled)
    bool enableDefaultConsoleOutput;     ///< Whether default console backend should output (when no backends specified)
    
    /**
     * @brief Constructor
     */
    LoggerConfig()
        : name("default"),
          level(LogLevel::INFO),
          asyncMode(true),
          asyncQueueSize(10000),
          autoFlush(false),
          flushIntervalSeconds(5),
          enableDefaultConsoleOutput(true) {}
};

/**
 * @brief Logger utility class, provides global logging functionality
 */
class RSLOG_API Logger {
public:
    /**
     * @brief Get the global Logger instance
     * @return Reference to the Logger singleton
     */
    static Logger& instance();
    
    /**
     * @brief Initialize the logging system
     * @param config Logger configuration
     * @return Whether initialization was successful
     */
    bool initialize(const LoggerConfig& config = LoggerConfig());
    
    /**
     * @brief Shutdown the logging system
     */
    void shutdown();
    
    /**
     * @brief Add a log backend
     * @param backend Log backend pointer
     */
    void addBackend(std::shared_ptr<LogBackend> backend);
    
    /**
     * @brief Remove a log backend by index
     * @param index Backend index
     * @return Whether removal was successful
     */
    bool removeBackend(size_t index);
    
    /**
     * @brief Get the number of backends
     * @return Number of backends
     */
    size_t getBackendCount() const;
    
    /**
     * @brief Set the log level
     * @param level Log level
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief Get the current log level
     * @return Current log level
     */
    LogLevel getLevel() const;
    
    /**
     * @brief Log a message
     * @param level Log level
     * @param message Log message
     * @param file Source file name
     * @param line Line number
     * @param function Function name
     */
    void log(LogLevel level, const std::string& message, 
             const std::string& file = "", int line = 0, 
             const std::string& function = "");
    
    /**
     * @brief Set async mode
     * @param async Whether to enable async mode
     * @param queueSize Async queue size
     */
    void setAsyncMode(bool async, size_t queueSize = 10000);
    
    /**
     * @brief Check if async mode is enabled
     * @return Whether async mode is enabled
     */
    bool isAsyncMode() const;
    
    /**
     * @brief Check if the logger is initialized
     * @return Whether the logger is initialized
     */
    bool isInitialized() const;
    
    /**
     * @brief Check if the specified log level will be logged
     * @param level Log level
     * @return Whether it will be logged
     */
    bool isLevelEnabled(LogLevel level) const;
    
    /**
     * @brief Get the logger name
     * @return Logger name
     */
    const std::string& getName() const;
    
    /**
     * @brief Manually flush all backends to ensure log messages are written to disk
     */
    void flush();
    
    /**
     * @brief Enable or disable automatic flush
     * @param enable Whether to enable automatic flush
     * @param intervalSeconds Auto flush interval in seconds (0 = disabled)
     */
    void setAutoFlush(bool enable, int intervalSeconds = 5);
    
    /**
     * @brief Check if automatic flush is enabled
     * @return Whether automatic flush is enabled
     */
    bool isAutoFlushEnabled() const;

private:
    /**
     * @brief Private constructor
     */
    Logger();
    
    /**
     * @brief Private destructor
     */
    ~Logger();
    
    /**
     * @brief Process log message (called by both sync/async modes)
     * @param message Log message
     */
    void processLogMessage(const LogMessage& message);
    
    /**
     * @brief Auto flush thread function
     */
    void autoFlushThreadFunc();
    
    /**
     * @brief Start auto flush thread
     */
    void startAutoFlushThread();
    
    /**
     * @brief Stop auto flush thread
     */
    void stopAutoFlushThread();

private:
    std::string name_;                              ///< Logger name
    LogLevel level_;                                ///< Current log level
    bool initialized_;                              ///< Initialization flag
    bool asyncMode_;                                ///< Async mode flag
    std::vector<std::shared_ptr<LogBackend>> backends_; ///< Backend list
    std::unique_ptr<LogQueue> logQueue_;            ///< Async log queue
    mutable std::mutex mutex_;                      ///< Mutex
    
    // Auto flush related members
    bool autoFlush_;                                ///< Auto flush enabled flag
    int flushIntervalSeconds_;                      ///< Auto flush interval in seconds
    std::unique_ptr<std::thread> flushThread_;      ///< Auto flush thread
    std::atomic<bool> flushThreadRunning_;          ///< Flush thread running flag
    std::condition_variable flushCondVar_;          ///< Condition variable for flush thread
};

/**
 * @brief Log stream class for stream-style logging
 */
class RSLOG_API LogStream {
public:
    /**
     * @brief Constructor
     * @param level Log level
     * @param file Source file name
     * @param line Line number
     * @param function Function name
     */
    LogStream(LogLevel level, const std::string& file, int line, const std::string& function);
    
    /**
     * @brief Destructor, outputs the log
     */
    ~LogStream();
    
    /**
     * @brief Stream output operator
     * @tparam T Data type
     * @param data Data to output
     * @return Stream reference, supports chaining
     */
    template<typename T>
    LogStream& operator<<(const T& data) {
        stream_ << data;
        return *this;
    }

private:
    LogLevel level_;            ///< Log level
    std::string file_;          ///< Source file name
    int line_;                  ///< Line number
    std::string function_;      ///< Function name
    std::ostringstream stream_; ///< String stream
};

} // namespace RSLog

//------------------------------------------------------------------------------
// Helper macros to simplify logging calls
//------------------------------------------------------------------------------

// Get current file name (without path)
#define RSLOG_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// Basic log macros
#define RSLOG_LOG(level, message) \
    do { \
        if (RSLog::Logger::instance().isLevelEnabled(level)) { \
            RSLog::Logger::instance().log(level, message, RSLOG_FILENAME, __LINE__, __FUNCTION__); \
        } \
    } while(0)

// Stream log macros
#define RSLOG_STREAM(level) \
    RSLog::LogStream(level, RSLOG_FILENAME, __LINE__, __FUNCTION__)

// Format log macros
#define RSLOG_FORMAT(level, format, ...) \
    do { \
        if (RSLog::Logger::instance().isLevelEnabled(level)) { \
            char buffer[4096]; \
            snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
            RSLog::Logger::instance().log(level, buffer, RSLOG_FILENAME, __LINE__, __FUNCTION__); \
        } \
    } while(0)

// Brace format log macros (using {} placeholders)
#define RSLOG_BRACE_FORMAT(level, ...) \
    do { \
        if (RSLog::Logger::instance().isLevelEnabled(level)) { \
            std::string formatted = RSLog::LogFormatter::format(__VA_ARGS__); \
            RSLog::Logger::instance().log(level, formatted, RSLOG_FILENAME, __LINE__, __FUNCTION__); \
        } \
    } while(0)

// Different level log macros - now unified to support both simple messages and brace formatting
// Uses brace format {} when additional arguments are provided, simple message otherwise

// Helper macros to handle both simple messages and formatted messages
#define RSLOG_DEBUG(...) \
    RSLOG_BRACE_FORMAT(RSLog::LogLevel::DEBUG, __VA_ARGS__)

#define RSLOG_INFO(...) \
    RSLOG_BRACE_FORMAT(RSLog::LogLevel::INFO, __VA_ARGS__)

#define RSLOG_WARN(...) \
    RSLOG_BRACE_FORMAT(RSLog::LogLevel::WARN, __VA_ARGS__)

#define RSLOG_ERROR(...) \
    RSLOG_BRACE_FORMAT(RSLog::LogLevel::ERROR, __VA_ARGS__)

#define RSLOG_FATAL(...) \
    RSLOG_BRACE_FORMAT(RSLog::LogLevel::FATAL, __VA_ARGS__)

// Different level stream log macros
#define RSLOG_DEBUG_STREAM   RSLOG_STREAM(RSLog::LogLevel::DEBUG)
#define RSLOG_INFO_STREAM    RSLOG_STREAM(RSLog::LogLevel::INFO)
#define RSLOG_WARN_STREAM    RSLOG_STREAM(RSLog::LogLevel::WARN)
#define RSLOG_ERROR_STREAM   RSLOG_STREAM(RSLog::LogLevel::ERROR)
#define RSLOG_FATAL_STREAM   RSLOG_STREAM(RSLog::LogLevel::FATAL)

//------------------------------------------------------------------------------
// Legacy format-specific macros (preserved for backward compatibility)
// NOTE: The base RSLOG_DEBUG/INFO/WARN/ERROR/FATAL macros now default to brace format {}.
// - Use _F variants for explicit printf-style formatting when needed
// - The _B variants are now aliases to base macros (redundant but kept for compatibility)
// - The _B_IF variants are now aliases to base _IF macros (redundant but kept for compatibility)
//------------------------------------------------------------------------------

// Different level format log macros (printf-style)
#define RSLOG_DEBUG_F(format, ...)   RSLOG_FORMAT(RSLog::LogLevel::DEBUG, format, ##__VA_ARGS__)
#define RSLOG_INFO_F(format, ...)    RSLOG_FORMAT(RSLog::LogLevel::INFO, format, ##__VA_ARGS__)
#define RSLOG_WARN_F(format, ...)    RSLOG_FORMAT(RSLog::LogLevel::WARN, format, ##__VA_ARGS__)
#define RSLOG_ERROR_F(format, ...)   RSLOG_FORMAT(RSLog::LogLevel::ERROR, format, ##__VA_ARGS__)
#define RSLOG_FATAL_F(format, ...)   RSLOG_FORMAT(RSLog::LogLevel::FATAL, format, ##__VA_ARGS__)

// Different level brace format log macros (using {} placeholders) - now same as base macros
#define RSLOG_DEBUG_B(...)   RSLOG_BRACE_FORMAT(RSLog::LogLevel::DEBUG, __VA_ARGS__)
#define RSLOG_INFO_B(...)    RSLOG_BRACE_FORMAT(RSLog::LogLevel::INFO, __VA_ARGS__)
#define RSLOG_WARN_B(...)    RSLOG_BRACE_FORMAT(RSLog::LogLevel::WARN, __VA_ARGS__)
#define RSLOG_ERROR_B(...)   RSLOG_BRACE_FORMAT(RSLog::LogLevel::ERROR, __VA_ARGS__)
#define RSLOG_FATAL_B(...)   RSLOG_BRACE_FORMAT(RSLog::LogLevel::FATAL, __VA_ARGS__)

// Conditional log macros
#define RSLOG_DEBUG_IF(condition, ...) \
    do { if (condition) { RSLOG_DEBUG(__VA_ARGS__); } } while(0)
#define RSLOG_INFO_IF(condition, ...) \
    do { if (condition) { RSLOG_INFO(__VA_ARGS__); } } while(0)
#define RSLOG_WARN_IF(condition, ...) \
    do { if (condition) { RSLOG_WARN(__VA_ARGS__); } } while(0)
#define RSLOG_ERROR_IF(condition, ...) \
    do { if (condition) { RSLOG_ERROR(__VA_ARGS__); } } while(0)
#define RSLOG_FATAL_IF(condition, ...) \
    do { if (condition) { RSLOG_FATAL(__VA_ARGS__); } } while(0)

// Conditional format log macros
#define RSLOG_DEBUG_F_IF(condition, format, ...) \
    do { if (condition) { RSLOG_DEBUG_F(format, ##__VA_ARGS__); } } while(0)
#define RSLOG_INFO_F_IF(condition, format, ...) \
    do { if (condition) { RSLOG_INFO_F(format, ##__VA_ARGS__); } } while(0)
#define RSLOG_WARN_F_IF(condition, format, ...) \
    do { if (condition) { RSLOG_WARN_F(format, ##__VA_ARGS__); } } while(0)
#define RSLOG_ERROR_F_IF(condition, format, ...) \
    do { if (condition) { RSLOG_ERROR_F(format, ##__VA_ARGS__); } } while(0)
#define RSLOG_FATAL_F_IF(condition, format, ...) \
    do { if (condition) { RSLOG_FATAL_F(format, ##__VA_ARGS__); } } while(0)

// Conditional brace format log macros - now aliases to base conditional macros
// Since base macros now use brace format by default, these are equivalent
#define RSLOG_DEBUG_B_IF(condition, ...) RSLOG_DEBUG_IF(condition, __VA_ARGS__)
#define RSLOG_INFO_B_IF(condition, ...) RSLOG_INFO_IF(condition, __VA_ARGS__)
#define RSLOG_WARN_B_IF(condition, ...) RSLOG_WARN_IF(condition, __VA_ARGS__)
#define RSLOG_ERROR_B_IF(condition, ...) RSLOG_ERROR_IF(condition, __VA_ARGS__)
#define RSLOG_FATAL_B_IF(condition, ...) RSLOG_FATAL_IF(condition, __VA_ARGS__)

// Conditional stream log macros that don't require copying LogStream
#define RSLOG_DEBUG_STREAM_IF(condition) \
    if (condition) RSLOG_DEBUG_STREAM
#define RSLOG_INFO_STREAM_IF(condition) \
    if (condition) RSLOG_INFO_STREAM
#define RSLOG_WARN_STREAM_IF(condition) \
    if (condition) RSLOG_WARN_STREAM
#define RSLOG_ERROR_STREAM_IF(condition) \
    if (condition) RSLOG_ERROR_STREAM
#define RSLOG_FATAL_STREAM_IF(condition) \
    if (condition) RSLOG_FATAL_STREAM

// Initialization macros
#define RSLOG_INIT(...) RSLog::Logger::instance().initialize(__VA_ARGS__)

// Shutdown macros
#define RSLOG_SHUTDOWN() RSLog::Logger::instance().shutdown()

// Flush macros
#define RSLOG_FLUSH() RSLog::Logger::instance().flush()

// Auto flush macros
#define RSLOG_SET_AUTO_FLUSH(enable, interval) RSLog::Logger::instance().setAutoFlush(enable, interval)
#define RSLOG_ENABLE_AUTO_FLUSH(interval) RSLog::Logger::instance().setAutoFlush(true, interval)
#define RSLOG_DISABLE_AUTO_FLUSH() RSLog::Logger::instance().setAutoFlush(false, 0)
