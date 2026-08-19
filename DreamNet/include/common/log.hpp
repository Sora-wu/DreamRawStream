//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <print>
#include <format>
#include <chrono>
#include <thread>
#include <string>
#include <string_view>
#include <cstdlib>

class Logger {
public:
    enum class Level {
        INFO,
        ERROR,
        FATAL,
        WARN,
        DEBUG
    };

    template<typename... Args>
    static void Log(Level level, const char* file, int line, std::string_view fmt, Args&&... args) {
#ifdef CONFIG_DEBUG
        // 允许输出
#else
        if (level == Level::DEBUG)
            return;
#endif

        // ---- 时间戳（精确到毫秒） ----
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::floor<std::chrono::milliseconds>(now);
        auto zt = std::chrono::zoned_time{std::chrono::current_zone(), now_ms};
        std::string timestamp = std::format("{:%Y-%m-%d %H:%M:%S}", zt);

        // ---- 级别字符串与颜色 ----
        std::string levelStr;
        const char* color = "";
        const char* reset = "\033[0m";
        switch (level) {
            case Level::INFO:  levelStr = "INFO";  break;
            case Level::ERROR: levelStr = "ERROR"; color = "\033[31m"; break; // 红色
            case Level::FATAL: levelStr = "FATAL"; color = "\033[31m"; break;
            case Level::WARN:  levelStr = "WARN";  color = "\033[33m"; break; // 黄色
            case Level::DEBUG: levelStr = "DEBUG"; color = "\033[32m"; break; // 绿色
        }

        // ---- 提取文件名（去掉路径） ----
        const std::string_view filePath(file);
        auto pos = filePath.find_last_of("/\\");
        std::string_view fileName = (pos != std::string_view::npos)
                                        ? filePath.substr(pos + 1)
                                        : filePath;

        // ---- 线程 ID ----
        std::string threadId = std::format("{}", std::this_thread::get_id());

        // ---- 格式化用户消息 ----
        std::string message = std::vformat(fmt, std::make_format_args(args...));

        // ---- 组装整条日志 ----
        std::string logLine = std::format("{}[{}] [{}] [{}] {} [{}:{}] {}",
                                          color, timestamp, levelStr, threadId,
                                          message, fileName, line, reset);

        std::println("{}", logLine);
        if (level == Level::FATAL) {
            std::exit(1);
        }
    }
};

#define LOG_INFO(fmt, ...) \
    Logger::Log(Logger::Level::INFO, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    Logger::Log(Logger::Level::ERROR, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

#define LOG_FATAL(fmt, ...) \
    Logger::Log(Logger::Level::FATAL, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    Logger::Log(Logger::Level::WARN, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

#ifndef CONFIG_DEBUG
    #define LOG_DEBUG(fmt, ...) ((void)0)
#else
    #define LOG_DEBUG(fmt, ...) \
        Logger::Log(Logger::Level::DEBUG, __FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#endif