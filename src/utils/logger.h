#pragma once

#include <cstdio>
#include <cstddef>

#include <Arduino.h>
#include <HardwareSerial.h>

#include "utils/system_time.h"

#define LOG_DISABLE

enum Severity
{
    LOG_ERROR = 0,
    LOG_WARNING = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3,
};

class Logger
{
    static constexpr size_t kSerialSpeedBaud = 115200;

public:
    explicit Logger(const char *class_name);

    static void SetMaxLevel(Severity level);
    static Severity GetMaxLevel(Severity);

    template <typename... Args>
    void LogFunc(const char *const function_name, Severity level, const char *format, Args... args) const
    {
#ifndef LOG_DISABLE
        if (level > max_level_)
        {
            return;
        }

        final_format_.clear();
        final_format_ += compound_format_;
        final_format_ += format;
        final_format_ += "\n";

        serial_->printf(final_format_.c_str(), SystemTime::GetUptimeString(), GetLogLevelText(level), function_name, args...);
#endif
    }

private:
    static void InitSerial();
    static const char *GetLogLevelText(Severity level);

    static int max_level_;
    static HardwareSerial *serial_;

#ifndef LOG_DISABLE
    String compound_format_;
    mutable String final_format_;
#endif
};

#define InternalLog(...) LogFunc(__func__, __VA_ARGS__)

#define LogDebug(...) InternalLog(LOG_DEBUG, __VA_ARGS__)
#define LogInfo(...) InternalLog(LOG_INFO, __VA_ARGS__)
#define LogWarning(...) InternalLog(LOG_WARNING, __VA_ARGS__)
#define LogError(...) InternalLog(LOG_ERROR, __VA_ARGS__)
