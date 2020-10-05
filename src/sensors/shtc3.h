#pragma once

#include <chrono>
#include <memory>

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>

#include <PubSubClient.h>
#include <SparkFun_SHTC3.h>

#include "homeassistant/sensor_device.h"

class Shtc3
{
public:
    Shtc3(HardwareSerial *serial, TwoWire *wire, PubSubClient *mqtt, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout);
    bool InitHardware();
    bool SendHomeassistantConfig();
    bool Loop();

    void SetBatteryVoltage(float voltage);

private:
    bool Update();

    bool WakeUp();
    void Sleep();

    SHTC3 device_;
    HardwareSerial *serial_;
    TwoWire *wire_;
    PubSubClient *mqtt_;

    std::shared_ptr<SensorDevice> ha_device_;
    std::shared_ptr<Sensor> ha_temperature_;
    std::shared_ptr<Sensor> ha_humidity_;
    std::shared_ptr<Sensor> ha_voltage_;
    std::shared_ptr<Sensor> ha_battery_;
};
