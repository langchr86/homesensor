#include "sensor.h"

#include <ArduinoJson.h>

static constexpr char kDeviceJsonObjectName[] = "device";

Sensor::Sensor(const String &readable_name, const String &unique_id, SensorDeviceClass device_class, const String &unit_of_measurement)
    : readable_name_(readable_name), unique_id_(unique_id),
      device_class_(device_class), unit_of_measurement_(unit_of_measurement), expire_timeout_(1), value_(0) {}

void Sensor::SetExpireTimeout(const std::chrono::seconds &timeout)
{
    expire_timeout_ = timeout;
}

void Sensor::SetValue(float value)
{
    value_ = value;
}

String Sensor::GetConfigPayload() const
{
    StaticJsonDocument<JSON_OBJECT_SIZE(32)> json;

    json["name"] = device_readable_name_ + " " + readable_name_;
    json["unique_id"] = device_unique_id_ + "_" + unique_id_;
    json["state_topic"] = device_state_topic_;

    json["device_class"] = ToString(device_class_);
    json["unit_of_measurement"] = unit_of_measurement_;
    json["value_template"] = "{{ value_json." + unique_id_ + " }}";
    // TODO(clang): json["icon"] = "";

    json["expire_after"] = static_cast<uint32_t>(expire_timeout_.count());

    auto device_json = json.createNestedObject(kDeviceJsonObjectName);
    device_json["name"] = device_readable_name_;
    device_json["identifiers"] = device_unique_id_;
    device_json["model"] = device_model_;
    device_json["manufacturer"] = device_manufacturer_;

    String output;
    serializeJson(json, output);
    return output;
}

String Sensor::GetUniqueId() const
{
    return unique_id_;
}

String Sensor::GetStateValue() const
{
    return String() + value_;
}

void Sensor::SetDeviceInfo(const String &readable_name, const String &unique_id,
                           const String &device_model, const String &device_manufacturer, const String &device_state_topic)
{
    device_readable_name_ = readable_name;
    device_unique_id_ = unique_id;
    device_model_ = device_model;
    device_manufacturer_ = device_manufacturer;
    device_state_topic_ = device_state_topic;
}
