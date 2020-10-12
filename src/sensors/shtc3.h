#pragma once

#include <chrono>
#include <memory>

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>

#include <PubSubClient.h>
#include <SparkFun_SHTC3.h>

#include "homeassistant/sensor_device.h"
#include "sensors/forward_declarations.h"
#include "sensors/sensor_base.h"

class Shtc3 : public SensorBase
{
public:
    Shtc3(HardwareSerial *serial, ADC *adc, TwoWire *wire, PubSubClient *mqtt, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout);
    bool InitHardware();
    bool Loop();

private:
    bool Update();

    bool WakeUp();
    void Sleep();

    SHTC3 device_;
    TwoWire *wire_;

    std::shared_ptr<Sensor> ha_temperature_;
    std::shared_ptr<Sensor> ha_humidity_;
};
