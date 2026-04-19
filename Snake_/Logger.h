#pragma once
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
//#include <iostream>

enum class LogLevel { INFO, WARN, ERR };

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void set_file(const std::string& path) {
        std::lock_guard<std::mutex> lock(mtx);
        if (file.is_open()) file.close();
        file.open(path, std::ios::ate);
    }

    void log(LogLevel level, const std::string& msg) {
        std::string line = "[" + timestamp() + "][" + level_str(level) + "] " + msg + "\n";

        std::lock_guard<std::mutex> lock(mtx);
        //std::cout << line;
        if (file.is_open())
            file << line << std::flush;
    }

private:
    Logger() = default;
    std::ofstream file;
    std::mutex mtx;

    std::string timestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        char buf[32];
        struct tm tm_info;
        localtime_s(&tm_info, &t);
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
        return buf;
    }

    std::string level_str(LogLevel l) {
        switch (l) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERR:  return "ERR ";
        }
        return "";
    }
};

#define LOG_INFO(msg) Logger::instance().log(LogLevel::INFO, msg)
#define LOG_WARN(msg) Logger::instance().log(LogLevel::WARN, msg)
#define LOG_ERR(msg)  Logger::instance().log(LogLevel::ERR,  msg)