//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include "RSLog/LogBackend.h"

#ifdef RSLOG_WITH_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>

namespace RSLog {

/**
 * @brief Spdlog backend adapter
 * 
 * This class adapts RSLog's logging interface to the spdlog library
 */
class SpdlogBackend : public LogBackend {
public:
    /**
     * @brief Constructor
     * @param config Backend configuration
     */
    explicit SpdlogBackend(const LogBackendConfig& config = LogBackendConfig());
    
    /**
     * @brief Destructor
     */
    ~SpdlogBackend() override;
    
    /**
     * @brief Initialize spdlog backend
     * @return Whether initialization was successful
     */
    bool initialize() override;
    
    /**
     * @brief Shutdown spdlog backend
     */
    void shutdown() override;
    
    /**
     * @brief Log using spdlog
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
     * @brief Convert RSLog log level to spdlog log level
     * @param level RSLog log level
     * @return spdlog log level
     */
    spdlog::level::level_enum convertLevel(LogLevel level) const;
    
private:
    std::shared_ptr<spdlog::logger> logger_; ///< spdlog logger instance
};

} // namespace RSLog
#endif // RSLOG_WITH_SPDLOG 