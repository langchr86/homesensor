#pragma once

#include <chrono>
#include <memory>

#include <Arduino.h>

#include <SoftWire.h>

#include <paulvha_SCD30.h>

#include "communication/connection.h"
#include "homeassistant/sensor_device.h"
#include "sensors/forward_declarations.h"
#include "sensors/sensor_base.h"

class Scd30 : public SensorBase
{
    static constexpr auto kMaxReadoutInterval = std::chrono::seconds(1800);
    static constexpr auto kMinReadoutInterval = std::chrono::seconds(2);
    static constexpr std::chrono::seconds kHardwareStartupDuration = std::chrono::seconds(2);
    static constexpr uint16_t kFreshAirReferencePpm = 400;

public:
    Scd30(ADC *adc, TwoWire *wire, Connection *connection, Power *power, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout);

    bool HardwareInitialization(const std::chrono::seconds &readout_interval) override;

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
