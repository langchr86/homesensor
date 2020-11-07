#include "sensor_base.h"

#include "battery_calculation.h"

SensorBase::SensorBase(ADC *adc, Connection *connection, Power *power, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout, const char *sensor_name)
    : logger_(sensor_name), connection_(connection), power_(power), adc_(adc), last_battery_level_(100)
{
    ha_device_ = std::make_shared<SensorDevice>(readable_name, unique_id, "firebeetle32", "espressif");

    ha_voltage_ = std::make_shared<Sensor>("Akkuspannung", "voltage", SensorDeviceClass::kNone, "V", "mdi:battery-heart-variant");
    ha_voltage_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_voltage_);

    ha_battery_ = std::make_shared<Sensor>("Akkukapazität", "capacity", SensorDeviceClass::kBattery, "%");
    ha_battery_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_battery_);

    ha_boot_count_ = std::make_shared<Sensor>("boot_count", "boot_count", SensorDeviceClass::kNone, "", "mdi:bell-check");
    ha_boot_count_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_boot_count_);

    ha_failed_boots_ = std::make_shared<Sensor>("failed_boots", "failed_boots", SensorDeviceClass::kNone, "", "mdi:bell-cancel");
    ha_failed_boots_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_failed_boots_);

    ha_max_readout_interval_ = std::make_shared<Sensor>("max_readout_interval", "max_readout_interval", SensorDeviceClass::kNone, "s", "mdi:bell-sleep");
    ha_max_readout_interval_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_max_readout_interval_);
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

bool SensorBase::LowPower() const
{
    return last_battery_level_ < 10;
}

void SensorBase::SetDebugInfos(size_t boot_count, size_t failed_boots, std::chrono::seconds max_readout_interval)
{
    logger_.LogInfo("boot_count=%u", boot_count);
    ha_boot_count_->SetValue(boot_count, 0);

    logger_.LogInfo("failed_boots=%u", failed_boots);
    ha_failed_boots_->SetValue(failed_boots, 0);

    logger_.LogInfo("max_readout_interval=%llus", max_readout_interval.count());
    ha_max_readout_interval_->SetValue(max_readout_interval.count(), 0);
}

void SensorBase::BatteryLoop()
{
    const auto voltage = adc_->ReadVoltage();
    last_battery_level_ = BatteryCalculation::CapacityLevelPercent(voltage);
    ha_battery_->SetValue(last_battery_level_, 0);
    ha_voltage_->SetValue(voltage, 2);
}
