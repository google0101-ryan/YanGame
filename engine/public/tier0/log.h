#pragma once

#include <print>
#include <ctime>
#include <exception>

#include <tier0/types.h>

enum LogLevel_t
{
    LL_TRACE,
    LL_INFO,
    LL_WARN,
    LL_ERROR,
    LL_FATAL
};

static str_t LevelNames[] =
{
    "TRACE",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

#define COLOR_RESET "\033[0m"
#define COLOR_TRACE "\033[38;5;33m"
#define COLOR_INFO "\033[38;5;46m"
#define COLOR_WARN "\033[38;5;226m"
#define COLOR_ERROR "\033[38;5;160m"
#define COLOR_FATAL "\033[38;5;160m"

#define LOG_LEVEL(x, level, ...) do { std::print("[{}]: " x, LevelNames[level], ##__VA_ARGS__); } while (0)

static void LogTimestamp()
{
    std::time_t t = std::time(0);
    std::tm* now = std::gmtime(&t);
    std::print("[{:04}-{:02}-{:02}-T{:02}:{:02}:{:02}]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
}

#define LOG_TRACE(x, ...) do { std::print(COLOR_TRACE); LogTimestamp(); LOG_LEVEL(x, LL_TRACE, ##__VA_ARGS__); std::print(COLOR_RESET); } while (0)
#define LOG_INFO(x, ...) do { std::print(COLOR_INFO); LogTimestamp(); LOG_LEVEL(x, LL_INFO, ##__VA_ARGS__); std::print(COLOR_RESET); } while (0)
#define LOG_WARN(x, ...) do { std::print(COLOR_WARN); LogTimestamp(); LOG_LEVEL(x, LL_WARN, ##__VA_ARGS__); std::print(COLOR_RESET); } while (0)
#define LOG_ERROR(x, ...) do { std::print(COLOR_ERROR); LogTimestamp(); LOG_LEVEL(x, LL_ERROR, ##__VA_ARGS__); std::print(COLOR_RESET); } while (0)
#define LOG_FATAL(x, ...) do { std::print(COLOR_FATAL); LogTimestamp(); LOG_LEVEL(x, LL_FATAL, ##__VA_ARGS__); std::print(COLOR_RESET); throw std::runtime_error("FATAL ERROR OCCURED!"); } while (0)