#pragma once

#include <chrono>

#include <Arduino.h>

#include "sensor_device_class.h"

class Sensor
{
public:
    Sensor(const String &readable_name, const String &unique_id, SensorDeviceClass device_class, const String &unit_of_measurement = "", const String &icon = "");

    void SetExpireTimeout(const std::chrono::seconds &timeout);
    void SetValue(float value);

    String GetConfigPayload() const;

    String GetUniqueId() const;
    String GetStateValue() const;

private:
    friend class SensorDevice;
    void SetDeviceInfo(const String &readable_name, const String &unique_id,
                       const String &device_model, const String &device_manufacturer, const String &device_state_topic);

    String readable_name_;
    String unique_id_;
    SensorDeviceClass device_class_;
    String unit_of_measurement_;
    String icon_;

    std::chrono::seconds expire_timeout_;
    float value_;

    String device_readable_name_;
    String device_unique_id_;
    String device_model_;
    String device_manufacturer_;
    String device_state_topic_;
};
