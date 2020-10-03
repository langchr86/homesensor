#include "auto_discovery.h"

const std::string AutoDiscovery::kTopicBase = "homeassistant/sensor/";

AutoDiscovery::AutoDiscovery()
{
}

void AutoDiscovery::SetUniqueName(const std::string &name)
{
    SetNameAndUniqueId(name, name);
}

void AutoDiscovery::SetNameAndUniqueId(const std::string &name, const std::string &unique_id)
{
    config_json_["name"] = name;
    config_json_["unique_id"] = unique_id;

    config_topic_ = kTopicBase + unique_id + "/config";
    state_topic_ = kTopicBase + unique_id + "/state";

    config_json_["state_topic"] = state_topic_;
}

void AutoDiscovery::SetAvailablitiy()
{
    /*
    if (this->availability_ == nullptr)
    {
        root_["availability_topic"] = global_mqtt_client->get_availability().topic;
        if (global_mqtt_client->get_availability().payload_available != "online")
            root_["payload_available"] = global_mqtt_client->get_availability().payload_available;
        if (global_mqtt_client->get_availability().payload_not_available != "offline")
            root_["payload_not_available"] = global_mqtt_client->get_availability().payload_not_available;
    }
    else if (!this->availability_->topic.empty())
    {
        root_["availability_topic"] = this->availability_->topic;
        if (this->availability_->payload_available != "online")
            root_["payload_available"] = this->availability_->payload_available;
        if (this->availability_->payload_not_available != "offline")
            root_["payload_not_available"] = this->availability_->payload_not_available;
    }
    */
}

void AutoDiscovery::SetDeviceInfo(const std::string &unique_device_id)
{
    auto device_info = config_json_.createNestedObject("device");
    device_info["identifiers"] = unique_device_id;
    device_info["name"] = unique_device_id;
    device_info["model"] = "firebeetle32";
    device_info["manufacturer"] = "espressif";
}

void AutoDiscovery::SetSensorData(SensorDeviceClass device_class, const std::string &unit_of_measurement)
{
    device_class_ = device_class;
    const auto class_string = std::to_string(device_class_);
    config_json_["device_class"] = class_string;
    config_json_["unit_of_measurement"] = unit_of_measurement;
    config_json_["value_template"] = "{{ value_json." + class_string + " }}";
    // root_["icon"] = "";
}

void AutoDiscovery::SetExpireTimeout(const std::chrono::seconds &timeout)
{
    config_json_["expire_after"] = static_cast<uint32_t>(timeout.count());
}

void AutoDiscovery::SetSensorValue(float value)
{
    const auto value_name = std::to_string(device_class_);
    state_json_[value_name] = value;
}

std::string AutoDiscovery::GetConfigTopic() const
{
    return config_topic_;
}

std::string AutoDiscovery::GetStateTopic() const
{
    return state_topic_;
}

std::string AutoDiscovery::GetConfigString() const
{
    std::string output;
    serializeJson(config_json_, output);
    return output;
}

std::string AutoDiscovery::GetStateString() const
{
    std::string output;
    serializeJson(state_json_, output);
    return output;
}
