#include "sensor_base.h"

#include "battery_calculation.h"

SensorBase::SensorBase(ADC *adc, Connection *connection, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout, const char *sensor_name)
    : logger_(sensor_name), connection_(connection), adc_(adc)
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
        if (connection_->Publish(message.GetTopic(), message.GetPayload()) == false)
        {
            logger_.LogError("Failed to publish config message to MQTT");
            return false;
        }

        logger_.LogInfo("Sent config message to: %s", message.GetTopic());
        logger_.LogInfo("  payload: %s", message.GetPayload());
    }

    return true;
}

bool SensorBase::SendHomeassistantState()
{
    const auto message = ha_device_->GetStateMessage();
    if (connection_->Publish(message.GetTopic(), message.GetPayload()) == false)
    {
        logger_.LogError("Failed to publish state message to MQTT");
        return false;
    }

    logger_.LogInfo("Sent state message to: %s", message.GetTopic());
    logger_.LogInfo("  payload: %s", message.GetPayload());

    return true;
}

bool SensorBase::SensorReadLoop()
{
    BatteryLoop();

    if (InternalPowerUp() == false)
    {
        logger_.LogError("Failed to wake up sensor");
        return false;
    }

    if (InternalSensorMeasurement() == false)
    {
        logger_.LogError("Failed to take sensor measurements");
        InternalPowerSave();
        InternalPowerDown();
        return false;
    }

    InternalPowerSave();

    if (InternalSensorRead() == false)
    {
        logger_.LogError("Failed to read sensor values");
        InternalPowerDown();
        return false;
    }

    InternalPowerDown();
    return true;
}

void SensorBase::BatteryLoop()
{
    const auto voltage = adc_->ReadVoltage();
    const auto battery_level = BatteryCalculation::CapacityLevelPercent(voltage);
    ha_battery_->SetValue(battery_level, 0);
    ha_voltage_->SetValue(voltage, 2);
}
