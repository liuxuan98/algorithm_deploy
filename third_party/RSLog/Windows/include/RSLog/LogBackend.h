//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include <string>
#include <memory>
#include <map>
#include "LogMessage.h"
#include "RSLogExport.h"

namespace RSLog {

/**
 * @brief Log backend configuration structure
 */
struct RSLOG_API LogBackendConfig {
    std::string logFilePath;         ///< Log file path
    bool enableConsoleOutput;        ///< Whether to output to console
    LogLevel minLevel;               ///< Minimum log level
    size_t maxFileSize;              ///< Maximum size of a single log file (bytes)
    size_t maxFiles;                 ///< Maximum number of log files (log rotation)
    std::string pattern;             ///< Log format pattern
    bool autoFlush;                  ///< Whether to enable automatic flush
    int flushIntervalSeconds;        ///< Auto flush interval in seconds (0 = disabled)
    std::map<std::string, std::string> extraOptions; ///< Additional backend-specific options

    /**
     * @brief Constructor, sets default values
     */
    LogBackendConfig()
        : logFilePath("logs/app.log"),
          enableConsoleOutput(true),
          minLevel(LogLevel::INFO),
          maxFileSize(10 * 1024 * 1024),  // 10MB
          maxFiles(5),
          pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] %v"),
          autoFlush(false),
          flushIntervalSeconds(5) {}
};

/**
 * @brief Log backend abstract base class
 * 
 * This class defines the common interface for log backends, all concrete log backend implementations should inherit from this class
 */
class RSLOG_API LogBackend {
public:
    /**
     * @brief Constructor
     * @param config Log backend configuration
     */
    explicit LogBackend(const LogBackendConfig& config = LogBackendConfig())
        : config_(config), initialized_(false) {}
    
    /**
     * @brief Virtual destructor
     */
    virtual ~LogBackend() = default;
    
    /**
     * @brief Initialize log backend
     * @return Whether initialization was successful
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Shutdown log backend
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Log a message
     * @param message Log message
     */
    virtual void log(const LogMessage& message) = 0;
    
    /**
     * @brief Flush I/O buffers to ensure all log messages are written to disk
     */
    virtual void flush() = 0;
    
    /**
     * @brief Set log level
     * @param level Minimum log level
     */
    virtual void setLevel(LogLevel level) {
        config_.minLevel = level;
    }
    
    /**
     * @brief Get current log level
     * @return Current log level
     */
    virtual LogLevel getLevel() const {
        return config_.minLevel;
    }
    
    /**
     * @brief Check if specified log level will be logged
     * @param level Log level to check
     * @return Whether it will be logged
     */
    virtual bool isLevelEnabled(LogLevel level) const {
        return level >= config_.minLevel;
    }
    
    /**
     * @brief Check if backend is initialized
     * @return Whether it is initialized
     */
    bool isInitialized() const {
        return initialized_;
    }
    
    /**
     * @brief Set log format pattern
     * @param pattern Log format pattern
     */
    virtual void setPattern(const std::string& pattern) {
        config_.pattern = pattern;
    }

    /**
     * @brief Get backend configuration
     * @return Constant reference to backend configuration
     */
    const LogBackendConfig& getConfig() const {
        return config_;
    }

    /**
     * @brief Set backend configuration
     * @param config New configuration
     */
    virtual void setConfig(const LogBackendConfig& config) {
        config_ = config;
    }

protected:
    LogBackendConfig config_;   ///< Log backend configuration
    bool initialized_;         ///< Initialization status flag
};

/**
 * @brief Create a log backend of specified type
 * @param backendType Backend type name ("glog", "spdlog", "log4cplus", etc.)
 * @param config Backend configuration
 * @return Smart pointer to log backend
 */
RSLOG_API std::shared_ptr<LogBackend> createLogBackend(const std::string& backendType, 
                                             const LogBackendConfig& config = LogBackendConfig());

} // namespace RSLog
