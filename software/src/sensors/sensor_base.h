#pragma once

#include <chrono>
#include <memory>

#include <Arduino.h>

#include "communication/connection.h"
#include "homeassistant/sensor_device.h"
#include "sensors/adc.h"
#include "sensors/sensor_interface.h"
#include "utils/logger.h"
#include "utils/power.h"

class SensorBase : public SensorInterface
{
public:
    SensorBase(ADC *adc, Connection *connection, Power *power, const char *readable_name, const char *unique_id, const char *sensor_name);

    bool SendHomeassistantConfig() override;
    bool SendHomeassistantState() override;

    bool SensorReadLoop() override;
    bool LowPower() const override;

    void SetDebugInfos(const char *mode, size_t boot_count, size_t failed_boots, std::chrono::seconds max_readout_interval) override;

protected:
    virtual bool InternalPowerUp() = 0;
    virtual bool InternalSensorMeasurement() = 0;
    virtual void InternalPowerSave() = 0;
    virtual bool InternalSensorRead() = 0;
    virtual void InternalPowerDown() = 0;

    Logger logger_;
    Connection *connection_;
    Power *power_;

    std::shared_ptr<SensorDevice> ha_device_;

private:
    void BatteryLoop();

    ADC *adc_;
    std::shared_ptr<Sensor> ha_voltage_;
    std::shared_ptr<Sensor> ha_battery_;
    std::shared_ptr<Sensor> ha_mode_;
    std::shared_ptr<Sensor> ha_boot_count_;
    std::shared_ptr<Sensor> ha_failed_boots_;
    std::shared_ptr<Sensor> ha_max_readout_interval_;

    float last_battery_level_;
};
