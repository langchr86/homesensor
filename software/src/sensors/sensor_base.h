#pragma once

#include <chrono>
#include <memory>

#include <Arduino.h>

#include "communication/connection.h"
#include "homeassistant/sensor_device.h"
#include "sensors/adc.h"
#include "utils/logger.h"

class SensorBase
{
public:
    SensorBase(ADC *adc, Connection *connection, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout, const char *sensor_name);

    bool SendHomeassistantConfig();
    bool SendHomeassistantState();

    virtual bool InitHardware() = 0;
    bool SensorReadLoop();

protected:
    virtual bool InternalPowerUp() = 0;
    virtual bool InternalSensorMeasurement() = 0;
    virtual void InternalPowerSave() = 0;
    virtual bool InternalSensorRead() = 0;
    virtual void InternalPowerDown() = 0;

    Logger logger_;
    Connection *connection_;

    std::shared_ptr<SensorDevice> ha_device_;

private:
    void BatteryLoop();

    ADC *adc_;
    std::shared_ptr<Sensor> ha_voltage_;
    std::shared_ptr<Sensor> ha_battery_;
};
