#include "base/logger.h"

#include <string>
#include <chrono>
#include <ctime>
#include <iostream>
#include <iomanip>

LogFileSink &LogFileSink::Instance() // 单例懒汉模式
{
    static LogFileSink sink;
    return sink;
}

LogFileSink::LogFileSink()
{
}
LogFileSink::~LogFileSink()
{
    DeInit();
}

void LogFileSink::Init(const std::string &filename, size_t max_size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    filename_ = filename;
    max_size_ = max_size;
    file_.open(filename_, std::ios::app);
}

void LogFileSink::DeInit()
{
    if (file_.is_open())
    {
        file_.close();
    }
}

void LogFileSink::Write(const std::string &message)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!file_.is_open())
    {
        return;
    }

    // 获取当前时间点
    time_t now = time(nullptr);
    struct tm *local = localtime(&now); // 注意：非线程安全[1,2](@ref) localtime_r()（Linux）或 localtime_s()（Windows）

    char time_buffer[22];
    std::strftime(time_buffer, sizeof(time_buffer), "[%Y-%m-%d %H:%M:%S]", local);
    // std::string time_str = std::string(time_buffer);
    size_t timestr_len = strlen(time_buffer) + 1;
    // size_t timestamp_len = time_str.size();

    if (current_size_ + timestr_len + message.size() >= max_size_)
    {
        RollFile();
    }

    // 写入带时间戳的日志
    auto pos = file_.tellp();                             // 获取当前写指针位置
    file_ << time_buffer << " " << message << std::flush; // 立即刷新确保日志不丢失
    auto new_pos = file_.tellp();
    current_size_ += static_cast<size_t>(new_pos - pos);
}

void LogFileSink::RollFile()
{
    DeInit();
    time_t ticks = time(NULL);
    struct tm *ptm = localtime(&ticks);
    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), ".%Y-%m-%d_%H-%M-%S", ptm);
    std::string new_name = filename_ + std::string(timestamp);
    std::rename(filename_.c_str(), new_name.c_str());
    file_.open(filename_, std::ios::app);
    current_size_ = 0;
}