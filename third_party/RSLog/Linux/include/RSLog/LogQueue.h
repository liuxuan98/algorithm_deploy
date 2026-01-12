//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <functional>
#include <memory>
#include "LogMessage.h"
#include "RSLogExport.h"

namespace RSLog {

/**
 * @brief Asynchronous log queue class
 * 
 * This class implements a thread-safe asynchronous log queue, used to cache log messages and process them asynchronously
 */
class RSLOG_API LogQueue {
public:
    using MessageProcessor = std::function<void(const LogMessage&)>;

    /**
     * @brief Constructor
     * @param processor Log message processor function
     * @param maxQueueSize Maximum queue capacity, 0 means unlimited
     */
    LogQueue(MessageProcessor processor, size_t maxQueueSize = 10000);

    /**
     * @brief Destructor, ensures all log messages are processed
     */
    ~LogQueue();

    /**
     * @brief Add a log message to the queue
     * @param message Log message
     * @return Whether successfully added
     */
    bool enqueue(const LogMessage& message);

    /**
     * @brief Start the asynchronous processing thread
     */
    void start();

    /**
     * @brief Stop the asynchronous processing thread
     * @param processRemaining Whether to process remaining messages in the queue
     */
    void stop(bool processRemaining = true);
    
    /**
     * @brief Get the current size of the queue
     * @return Number of messages in the queue
     */
    size_t size() const;
    
    /**
     * @brief Check if the queue is empty
     * @return Whether the queue is empty
     */
    bool empty() const;
    
    /**
     * @brief Clear the queue
     */
    void clear();
    
    /**
     * @brief Set the maximum queue capacity
     * @param maxSize Maximum capacity, 0 means unlimited
     */
    void setMaxQueueSize(size_t maxSize);
    
    /**
     * @brief Get the maximum queue capacity
     * @return Maximum queue capacity
     */
    size_t getMaxQueueSize() const;

private:
    /**
     * @brief Worker thread function, responsible for processing messages in the queue
     */
    void processQueue();

private:
    std::queue<LogMessage> queue_;                ///< Log message queue
    mutable std::mutex mutex_;                    ///< Queue mutex
    std::condition_variable condVar_;             ///< Condition variable for thread synchronization
    std::atomic<bool> running_;                   ///< Thread running status flag
    std::unique_ptr<std::thread> workerThread_;   ///< Worker thread
    MessageProcessor processor_;                  ///< Message processor function
    size_t maxQueueSize_;                         ///< Maximum queue capacity
};

} // namespace RSLog
