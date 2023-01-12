#pragma once

#include <chrono>

class SensorInterface
{
public:
    virtual ~SensorInterface() = default;

    virtual bool SendHomeassistantConfig() = 0;
    virtual bool SendHomeassistantState() = 0;

    virtual bool HardwareInitialization(const std::chrono::seconds &readout_interval) = 0;
    virtual bool ForceCalibrationNow() = 0;
    virtual bool SensorReadLoop() = 0;
    virtual bool LowPower() const = 0;

    virtual void SetDebugInfos(const char *mode, size_t boot_count, size_t failed_boots, std::chrono::seconds max_readout_interval, size_t ha_config_count) = 0;
};
