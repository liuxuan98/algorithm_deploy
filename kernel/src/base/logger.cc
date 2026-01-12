#include "base/logger.h"
// NOLINTBEGIN
#include "RSLog/Log.h"
// NOLINTEND

#include <array>
#include <cstring>
#include <string>
#include <chrono>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>

// Constants for configuration
namespace
{
    constexpr size_t DEFAULT_ASYNC_QUEUE_SIZE = 1000;
    constexpr size_t DEFAULT_MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB
    constexpr size_t TIME_BUFFER_SIZE = 32;
    constexpr int FLUSH_INTERVAL_SECONDS = 2;
} // namespace

// Global RSLog initialization flag
static bool g_rslog_initialized = false;
static std::mutex g_rslog_mutex;

// RSLog system initialization
void InitRSLogSystem() {
    std::lock_guard<std::mutex> lock(g_rslog_mutex);
    if (!g_rslog_initialized) {
        // Initialize RSLog with configuration
        RSLog::LoggerConfig config;
        config.name = "RayShape";
        config.level = RSLog::LogLevel::DEBUG;
        config.asyncMode = true;
        config.asyncQueueSize = DEFAULT_ASYNC_QUEUE_SIZE;
        config.autoFlush = true;
        config.flushIntervalSeconds = FLUSH_INTERVAL_SECONDS;

        RSLog::Logger &logger = RSLog::Logger::instance();
        if (logger.initialize(config)) {
            g_rslog_initialized = true;
        }
    }
}

void ShutdownRSLogSystem() {
    std::lock_guard<std::mutex> lock(g_rslog_mutex);
    if (g_rslog_initialized) {
        RSLog::Logger::instance().shutdown();
        g_rslog_initialized = false;
    }
}

// RSLog wrapper functions
void RSLogDebug(const std::string &message) {
    if (g_rslog_initialized) {
        RSLog::Logger::instance().log(RSLog::LogLevel::DEBUG, message);
    } else {
        std::cout << "[DEBUG] " << message << std::endl;
    }
}

void RSLogInfo(const std::string &message) {
    if (g_rslog_initialized) {
        RSLog::Logger::instance().log(RSLog::LogLevel::INFO, message);
    } else {
        std::cout << "[INFO] " << message << std::endl;
    }
}

void RSLogWarn(const std::string &message) {
    if (g_rslog_initialized) {
        RSLog::Logger::instance().log(RSLog::LogLevel::WARN, message);
    } else {
        std::cout << "[WARN] " << message << std::endl;
    }
}

void RSLogError(const std::string &message) {
    if (g_rslog_initialized) {
        RSLog::Logger::instance().log(RSLog::LogLevel::ERROR, message);
    } else {
        std::cerr << "[ERROR] " << message << std::endl;
    }
}

// Legacy LogFileSink implementation for backward compatibility
LogFileSink &LogFileSink::Instance() {
    static LogFileSink sink;
    return sink;
}

LogFileSink::LogFileSink() : max_size_(DEFAULT_MAX_FILE_SIZE) {
    // Initialize RSLog system when LogFileSink is first accessed
    InitRSLogSystem();
}

LogFileSink::~LogFileSink() {
    DeInit();
}

void LogFileSink::Init(const std::string &filename, size_t max_size) {
    filename_ = filename;
    max_size_ = max_size;
    initialized_ = true;
}

void LogFileSink::DeInit() {
    initialized_ = false;
}

void LogFileSink::Write(const std::string &message) {
    if (!initialized_) {
        return;
    }

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto local_time = std::localtime(&time_t);

    std::array<char, TIME_BUFFER_SIZE> time_buffer{};
    std::strftime(time_buffer.data(), time_buffer.size(), "[%Y-%m-%d %H:%M:%S]", local_time);

    // Send to RSLog
    std::string full_message = std::string(time_buffer.data()) + " " + message;
    RSLogInfo(full_message);

    // Update current size (approximate)
    current_size_ += full_message.size();

    // Check if we need to roll file
    if (current_size_ >= max_size_) {
        RollFile();
    }
}

void LogFileSink::RollFile() {
    // Reset our counter
    current_size_ = 0;
}
