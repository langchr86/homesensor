#pragma once

#include <chrono>
#include <memory>

#include <Arduino.h>

#include <PubSubClient.h>

#include "homeassistant/sensor_device.h"
#include "sensors/adc.h"
#include "utils/logger.h"

class SensorBase
{
public:
    SensorBase(ADC *adc, PubSubClient *mqtt, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout, const char *sensor_name);

    bool SendHomeassistantConfig();

    bool Loop();

protected:
    void BatteryLoop();

    virtual bool InternalLoop() = 0;

    Logger logger_;
    PubSubClient *mqtt_;

    std::shared_ptr<SensorDevice> ha_device_;

private:
    ADC *adc_;
    std::shared_ptr<Sensor> ha_voltage_;
    std::shared_ptr<Sensor> ha_battery_;
};
