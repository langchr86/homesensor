#include "logger.h"

int Logger::max_level_ = LOG_DEBUG;
HardwareSerial *Logger::serial_ = &Serial;

Logger::Logger(const char *class_name)
{
#ifndef LOG_DISABLE
    compound_format_ += "%s [%s][";
    compound_format_ += class_name;
    compound_format_ += "::%s";
    compound_format_ += "] ";

    InitSerial();
#endif
}

void Logger::SetMaxLevel(Severity level)
{
    max_level_ = level;
}

Severity Logger::GetMaxLevel(Severity)
{
    return static_cast<Severity>(max_level_);
}

void Logger::InitSerial()
{
    static bool initialized = false;
    if (initialized)
    {
        return;
    }

    serial_->begin(kSerialSpeedBaud);
    delay(1000);
    serial_->printf("%s serial interface initialized\n", SystemTime::GetUptimeString());

    initialized = true;
}

const char *Logger::GetLogLevelText(Severity level)
{
    switch (level)
    {
    case LOG_ERROR:
        return "ERR ";
    case LOG_WARNING:
        return "WARN";
    case LOG_INFO:
        return "INFO";
    case LOG_DEBUG:
        return "DEBG";
    default:
        return "UNKN";
    }
}
