#include "sensor_base.h"

#include "battery_calculation.h"

SensorBase::SensorBase(ADC *adc, PubSubClient *mqtt, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout, const char *sensor_name)
    : logger_(sensor_name), mqtt_(mqtt), adc_(adc)
{
    ha_device_ = std::make_shared<SensorDevice>(readable_name, unique_id, "firebeetle32", "espressif");

    ha_voltage_ = std::make_shared<Sensor>("Akkuspannung", "voltage", SensorDeviceClass::kNone, "V", "mdi:battery-heart-variant");
    ha_voltage_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_voltage_);

    ha_battery_ = std::make_shared<Sensor>("Akkukapazität", "capacity", SensorDeviceClass::kBattery, "%");
    ha_battery_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_battery_);
}

bool SensorBase::SendHomeassistantConfig()
{
    for (const auto &message : ha_device_->GetAllConfigMessages())
    {
        if (mqtt_->publish(message.GetTopic(), message.GetPayload()) == false)
        {
            logger_.LogError("Failed to publish initial HA config messages to MQTT");
            return false;
        }

        logger_.LogDebug("Sent HA config message to: %s", message.GetTopic());
        logger_.LogDebug("  payload: %s", message.GetPayload());
    }

    return true;
}

bool SensorBase::Loop()
{
    BatteryLoop();
    return InternalLoop();
}

void SensorBase::BatteryLoop()
{
    const auto voltage = adc_->ReadVoltage();
    const auto battery_level = BatteryCalculation::CapacityLevelPercent(voltage);
    ha_battery_->SetValue(battery_level, 0);
    ha_voltage_->SetValue(voltage, 2);
}
