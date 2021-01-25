#include "sensor_device.h"

#include <ArduinoJson.h>

const constexpr char kSensorTopicBase[] = "homeassistant/sensor/";

SensorDevice::SensorDevice(const String &readable_name, const String &unique_id, const String &model, const String &manufacturer)
    : readable_name_(readable_name), unique_id_(unique_id), model_(model), manufacturer_(manufacturer) {}

void SensorDevice::AddSensor(const SensorPtr &sensor)
{
    sensor->SetDeviceInfo(readable_name_, unique_id_, model_, manufacturer_, CreateDeviceStateTopic());
    sensors_.push_back(sensor);
}

String SensorDevice::GetUniqueId() const
{
    return unique_id_;
}

std::vector<MqttMessage> SensorDevice::GetAllConfigMessages() const
{
    std::vector<MqttMessage> messages;
    for (const auto &sensor : sensors_)
    {
        messages.emplace_back(CreateSensorConfigTopic(sensor->GetUniqueId()), sensor->GetConfigPayload());
    }

    return messages;
}

MqttMessage SensorDevice::GetStateMessage() const
{
    StaticJsonDocument<JSON_OBJECT_SIZE(20)> json;
    for (const auto &sensor : sensors_)
    {
        json[sensor->GetUniqueId()] = sensor->GetStateValue();
    }

    String payload;
    serializeJson(json, payload);
    return MqttMessage(CreateDeviceStateTopic(), payload);
}

String SensorDevice::CreateSensorConfigTopic(const String &sensor_unique_id) const
{
    return kSensorTopicBase + unique_id_ + "_" + sensor_unique_id + "/config";
}

String SensorDevice::CreateDeviceStateTopic() const
{
    return kSensorTopicBase + unique_id_ + "/state";
}
