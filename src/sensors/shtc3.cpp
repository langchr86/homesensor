#include "shtc3.h"

Shtc3::Shtc3(HardwareSerial *logger, TwoWire *wire, PubSubClient *mqtt, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout)
    : serial_(logger), wire_(wire), mqtt_(mqtt)
{
    ha_device_ = std::make_shared<SensorDevice>(readable_name, unique_id, "firebeetle32", "espressif");

    ha_temperature_ = std::make_shared<Sensor>("Temperatur", "temperature", SensorDeviceClass::kTemperature, "°C");
    ha_temperature_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_temperature_);

    ha_humidity_ = std::make_shared<Sensor>("Feuchtigkeit", "humidity", SensorDeviceClass::kHumidity, "%");
    ha_humidity_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_humidity_);
}

bool Shtc3::InitHardware()
{
    if (device_.begin(*wire_) != SHTC3_Status_Nominal)
    {
        serial_->println("Failed to setup wire connection to sensor");
        return false;
    }

    if (device_.passIDcrc == false)
    {
        serial_->println("ID checksum failed");
        return false;
    }
    else
    {
        serial_->print("ID checksum passed. Device ID: 0b");
        serial_->println(device_.ID, BIN);
    }

    if (device_.setMode(SHTC3_CMD_CSE_TF_LPM) != SHTC3_Status_Nominal)
    {
        serial_->println("Failed to set sensor mode");
        return false;
    }

    return true;
}

bool Shtc3::SendHomeassistantConfig()
{
    for (const auto &message : ha_device_->GetAllConfigMessages())
    {
        if (mqtt_->publish(message.GetTopic(), message.GetPayload()) == false)
        {
            serial_->println("Failed to publish initial HA config messages to MQTT");
            return false;
        }

        serial_->print("Sent HA config message to: ");
        serial_->println(message.GetTopic());
        serial_->println(message.GetPayload());
        serial_->println("-------------------");
    }

    return true;
}

bool Shtc3::Loop()
{
    if (WakeUp() == false)
    {
        return false;
    }

    if (Update() == false)
    {
        Sleep();
        return false;
    }

    Sleep();

    if (device_.passRHcrc == false)
    {
        serial_->println("Sensor CRC failed: Humidity");
    }
    if (device_.passTcrc == false)
    {
        serial_->println("Sensor CRC failed: Temperature");
    }

    ha_temperature_->SetValue(device_.toDegC());
    ha_humidity_->SetValue(device_.toPercent());

    const auto message = ha_device_->GetStateMessage();
    if (mqtt_->publish(message.GetTopic(), message.GetPayload()) == false)
    {
        serial_->println("Failed to publish state message to MQTT");
        return false;
    }

    serial_->print("Sent state message to: ");
    serial_->println(message.GetTopic());
    serial_->println(message.GetPayload());
    serial_->println("-------------------");

    return true;
}

bool Shtc3::Update()
{
    if (device_.update() != SHTC3_Status_Nominal)
    {
        serial_->println("Failed read out sensor values");
        return false;
    }
    return true;
}

bool Shtc3::WakeUp()
{
    if (device_.wake(true) != SHTC3_Status_Nominal)
    {
        serial_->println("Failed to wake up sensor");
        return false;
    }
    return true;
}

void Shtc3::Sleep()
{
    device_.sleep(true);
}
