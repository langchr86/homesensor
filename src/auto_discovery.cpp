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
    root_["name"] = name;
    root_["unique_id"] = unique_id;

    config_topic_ = kTopicBase + unique_id + "/config";
    state_topic_ = kTopicBase + unique_id + "/state";

    root_["state_topic"] = state_topic_;
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
    auto device_info = root_.createNestedObject("device");
    device_info["identifiers"] = unique_device_id;
    device_info["name"] = unique_device_id;
    device_info["model"] = "firebeetle32";
    device_info["manufacturer"] = "espressif";
}

void AutoDiscovery::SetSensorData(SensorDeviceClass device_class, const std::string &unit_of_measurement)
{
    root_["device_class"] = std::to_string(device_class);
    root_["unit_of_measurement"] = unit_of_measurement;
    // root_["icon"] = "";
}

void AutoDiscovery::SetExpireTimeout(const std::chrono::seconds &timeout)
{
    root_["expire_after"] = static_cast<uint32_t>(timeout.count());
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
    serializeJson(root_, output);
    return output;
}
