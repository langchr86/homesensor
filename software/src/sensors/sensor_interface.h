#pragma once

class SensorInterface
{
public:
    virtual ~SensorInterface() = default;

    virtual bool SendHomeassistantConfig() = 0;
    virtual bool SendHomeassistantState() = 0;

    virtual bool SensorReadLoop() = 0;

    virtual bool LowPower() const = 0;

    virtual void SetDebugInfos(size_t boot_count, size_t failed_boots, std::chrono::seconds max_readout_interval) = 0;
};
