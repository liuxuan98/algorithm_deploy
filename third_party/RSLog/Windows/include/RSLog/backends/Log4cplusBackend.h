//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include "RSLog/LogBackend.h"

#ifdef RSLOG_WITH_LOG4CPLUS
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include <memory>

namespace RSLog {

/**
 * @brief Log4cplus backend adapter
 * 
 * This class adapts RSLog's logging interface to the log4cplus library
 */
class Log4cplusBackend : public LogBackend {
public:
    /**
     * @brief Constructor
     * @param config Backend configuration
     */
    explicit Log4cplusBackend(const LogBackendConfig& config = LogBackendConfig());
    
    /**
     * @brief Destructor
     */
    ~Log4cplusBackend() override;
    
    /**
     * @brief Initialize log4cplus backend
     * @return Whether initialization was successful
     */
    bool initialize() override;
    
    /**
     * @brief Shutdown log4cplus backend
     */
    void shutdown() override;
    
    /**
     * @brief Log using log4cplus
     * @param message Log message
     */
    void log(const LogMessage& message) override;
    
    /**
     * @brief Set log level
     * @param level Minimum log level
     */
    void setLevel(LogLevel level) override;
    
    /**
     * @brief Set log format pattern
     * @param pattern Log format pattern
     */
    void setPattern(const std::string& pattern) override;
    
    /**
     * @brief Flush I/O buffers to ensure all log messages are written to disk
     */
    void flush() override;

private:
    /**
     * @brief Convert RSLog log level to log4cplus log level
     * @param level RSLog log level
     * @return log4cplus log level
     */
    log4cplus::LogLevel convertLevel(LogLevel level) const;
    
    /**
     * @brief Configure log4cplus
     * @return Whether configuration was successful
     */
    bool configureLog4cplus();

private:
    log4cplus::Logger logger_;      ///< log4cplus logger instance
    std::string loggerName_;        ///< Logger name
    std::string layoutPattern_;     ///< Log format pattern
};

} // namespace RSLog
#endif // RSLOG_WITH_LOG4CPLUS 