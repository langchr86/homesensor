#pragma once

#include <chrono>

#include <Arduino.h>

#include "homeassistant/sensor_device_class.h"
#include "utils/logger.h"

class Sensor
{
public:
    Sensor(const String &readable_name, const String &unique_id, SensorDeviceClass device_class, const String &unit_of_measurement = "", const String &icon = "");

    void SetExpireTimeout(const std::chrono::seconds &timeout);
    void SetValue(String value_string);
    void SetValue(float value, size_t decimal_precision = 1);
    void SetCalibration(float measurement_low, float measurement_high, float expected_low, float expected_high);

    String GetConfigPayload() const;

    String GetUniqueId() const;
    String GetStateValue() const;

private:
    friend class SensorDevice;
    void SetDeviceInfo(const String &readable_name, const String &unique_id,
                       const String &device_model, const String &device_manufacturer, const String &device_state_topic);

    Logger logger_;

    String readable_name_;
    String unique_id_;
    SensorDeviceClass device_class_;
    String unit_of_measurement_;
    String icon_;

    std::chrono::seconds expire_timeout_;
    String value_;

    float gain_;
    float offset_;

    String device_readable_name_;
    String device_unique_id_;
    String device_model_;
    String device_manufacturer_;
    String device_state_topic_;
};
