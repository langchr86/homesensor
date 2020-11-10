#pragma once

class SensorInterface
{
public:
    virtual ~SensorInterface() = default;

    virtual bool SendHomeassistantConfig() = 0;
    virtual bool SendHomeassistantState() = 0;

    virtual bool InitHardware() = 0;
    virtual bool SensorReadLoop() = 0;
};
