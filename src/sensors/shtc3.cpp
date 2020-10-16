#include "shtc3.h"

Shtc3::Shtc3(ADC *adc, TwoWire *wire, PubSubClient *mqtt, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout)
    : SensorBase(adc, mqtt, readable_name, unique_id, expire_timeout, "SHTC3"), wire_(wire)
{
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
        logger_.LogError("Failed to setup wire connection to sensor");
        return false;
    }

    if (device_.passIDcrc == false)
    {
        logger_.LogError("ID checksum failed");
        return false;
    }

    if (device_.setMode(SHTC3_CMD_CSE_TF_LPM) != SHTC3_Status_Nominal)
    {
        logger_.LogError("Failed to set sensor mode");
        return false;
    }

    return true;
}

bool Shtc3::InternalLoop()
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
        logger_.LogError("Sensor CRC failed: Humidity");
    }
    if (device_.passTcrc == false)
    {
        logger_.LogError("Sensor CRC failed: Temperature");
    }

    ha_temperature_->SetValue(device_.toDegC());
    ha_humidity_->SetValue(device_.toPercent(), 0);

    const auto message = ha_device_->GetStateMessage();
    if (mqtt_->publish(message.GetTopic(), message.GetPayload()) == false)
    {
        logger_.LogError("Failed to publish state message to MQTT");
        return false;
    }

    logger_.LogDebug("Sent state message to: %s", message.GetTopic());
    logger_.LogDebug("  payload: %s", message.GetPayload());

    return true;
}

bool Shtc3::Update()
{
    if (device_.update() != SHTC3_Status_Nominal)
    {
        logger_.LogError("Failed read out sensor values");
        return false;
    }
    return true;
}

bool Shtc3::WakeUp()
{
    if (device_.wake(true) != SHTC3_Status_Nominal)
    {
        logger_.LogError("Failed to wake up sensor");
        return false;
    }
    return true;
}

void Shtc3::Sleep()
{
    device_.sleep(true);
}
