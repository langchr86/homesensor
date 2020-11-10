#pragma once

#include <chrono>
#include <memory>

#include <Arduino.h>
#include <Wire.h>

#include <SparkFun_SCD30_Arduino_Library.h>

#include "communication/connection.h"
#include "homeassistant/sensor_device.h"
#include "sensors/forward_declarations.h"
#include "sensors/sensor_base.h"

class Scd30 : public SensorBase
{
public:
    Scd30(ADC *adc, TwoWire *wire, Connection *connection, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout);
    bool InitHardware();

private:
    bool InternalPowerUp() override;
    bool InternalSensorMeasurement() override;
    void InternalPowerSave() override;
    bool InternalSensorRead() override;
    void InternalPowerDown() override;

    SCD30 device_;
    TwoWire *wire_;

    std::shared_ptr<Sensor> ha_temperature_;
    std::shared_ptr<Sensor> ha_humidity_;
    std::shared_ptr<Sensor> ha_co2_;
};
