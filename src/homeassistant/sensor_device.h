#pragma once

#include <memory>
#include <vector>

#include <Arduino.h>

#include "communication/mqtt_message.h"
#include "homeassistant/sensor.h"
#include "homeassistant/sensor_device_class.h"

class SensorDevice
{
    static const String kTopicBase;

public:
    using SensorPtr = std::shared_ptr<Sensor>;

    SensorDevice(const String &readable_name, const String &unique_id, const String &model, const String &manufacturer);

    void AddSensor(const SensorPtr &sensor);

    String GetUniqueId() const;

    std::vector<MqttMessage> GetAllConfigMessages() const;
    MqttMessage GetStateMessage() const;

private:
    String CreateSensorConfigTopic(const String& unique_name) const;
    String CreateDeviceStateTopic() const;

    String readable_name_;
    String unique_id_;
    String model_;
    String manufacturer_;

    std::vector<SensorPtr> sensors_;
};
