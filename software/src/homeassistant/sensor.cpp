#include "sensor.h"

#include <ArduinoJson.h>

static constexpr char kDeviceJsonObjectName[] = "device";

Sensor::Sensor(const String &readable_name, const String &unique_id, SensorDeviceClass device_class, const String &unit_of_measurement, const String &icon)
    : logger_(unique_id.c_str()), readable_name_(readable_name), unique_id_(unique_id),
      device_class_(device_class), unit_of_measurement_(unit_of_measurement), icon_(icon), expire_timeout_(0), value_(0), gain_(0), offset_(0) {}

void Sensor::SetExpireTimeout(const std::chrono::seconds &timeout)
{
    expire_timeout_ = timeout;
}

void Sensor::SetValue(String value_string)
{
    value_string.trim();
    value_ = value_string;
}

void Sensor::SetValue(float value, size_t decimal_precision)
{
    if (gain_ != 0)
    {
        value = value * gain_ + offset_;
    }

    String value_string(value, decimal_precision);
    SetValue(value_string);
}

void Sensor::SetCalibration(float measurement_low, float measurement_high, float expected_low, float expected_high)
{
    gain_ = (expected_high - expected_low) / (measurement_high - measurement_low);
    offset_ = expected_low - measurement_low * gain_;
    logger_.LogInfo("Calibrated: gain=%f offset=%f", gain_, offset_);
}

String Sensor::GetConfigPayload() const
{
    StaticJsonDocument<JSON_OBJECT_SIZE(64)> json;

    json["name"] = readable_name_;
    json["unique_id"] = device_unique_id_ + "_" + unique_id_;
    json["state_topic"] = device_state_topic_;
    json["value_template"] = "{{ value_json." + unique_id_ + " }}";
    json["state_class"] = "measurement";

    if (device_class_ != SensorDeviceClass::kNone)
    {
        json["device_class"] = ToString(device_class_);
    }
    if (unit_of_measurement_.isEmpty() == false)
    {
        json["unit_of_measurement"] = unit_of_measurement_;
    }
    if (icon_.isEmpty() == false)
    {
        json["icon"] = icon_;
    }
    if (expire_timeout_ != std::chrono::seconds::zero())
    {
        json["expire_after"] = static_cast<uint32_t>(expire_timeout_.count());
    }

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
    return value_;
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
