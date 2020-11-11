#pragma once

#include <chrono>
#include <memory>

#include <Arduino.h>
#include <Wire.h>

#include "infrastructure/device_config.h"
#include "sensors/sensor_interface.h"

#include "communication/connection.h"
#include "sensors/adc.h"
#include "utils/logger.h"
#include "utils/power.h"

class DeviceFactory
{
public:
    DeviceFactory(const std::chrono::seconds &readout_interval, const std::chrono::seconds &expire_timeout);

    std::shared_ptr<SensorInterface> CreateDevice(const DeviceConfig &config, ADC *adc, TwoWire *wire, Connection *connection, Power *power);

private:
    Logger logger_;
    std::chrono::seconds readout_interval_;
    std::chrono::seconds expire_timeout_;
};
