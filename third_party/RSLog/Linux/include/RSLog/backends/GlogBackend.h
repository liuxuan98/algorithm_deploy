//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include "RSLog/LogBackend.h"

#ifdef RSLOG_WITH_GLOG
#include <glog/logging.h>
#include <memory>

namespace RSLog {

/**
 * @brief Google glog backend adapter
 * 
 * This class adapts RSLog's logging interface to the Google glog library
 */
class GlogBackend : public LogBackend {
public:
    /**
     * @brief Constructor
     * @param config Backend configuration
     */
    explicit GlogBackend(const LogBackendConfig& config = LogBackendConfig());
    
    /**
     * @brief Destructor
     */
    ~GlogBackend() override;
    
    /**
     * @brief Initialize glog backend
     * @return Whether initialization was successful
     */
    bool initialize() override;
    
    /**
     * @brief Shutdown glog backend
     */
    void shutdown() override;
    
    /**
     * @brief Log using glog
     * @param message Log message
     */
    void log(const LogMessage& message) override;
    
    /**
     * @brief Set log level
     * @param level Minimum log level
     */
    void setLevel(LogLevel level) override;
    
    /**
     * @brief Flush I/O buffers to ensure all log messages are written to disk
     */
    void flush() override;

private:
    /**
     * @brief Convert RSLog log level to glog log level
     * @param level RSLog log level
     * @return glog log level
     */
    int convertLevel(LogLevel level) const;
    
private:
    bool initialized_internally_; ///< Whether glog was initialized by this class
};

} // namespace RSLog
#endif // RSLOG_WITH_GLOG 