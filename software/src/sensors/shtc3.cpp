#include "shtc3.h"

Shtc3::Shtc3(ADC *adc, TwoWire *wire, Connection *connection, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout)
    : SensorBase(adc, connection, readable_name, unique_id, expire_timeout, "SHTC3"), wire_(wire)
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

bool Shtc3::InternalPowerUp()
{
    return device_.wake(true) == SHTC3_Status_Nominal;
}

bool Shtc3::InternalSensorMeasurement()
{
    return device_.update() == SHTC3_Status_Nominal;
}

void Shtc3::InternalPowerSave()
{
    device_.sleep(true);
}

bool Shtc3::InternalSensorRead()
{
    if (device_.passRHcrc == false)
    {
        logger_.LogError("Sensor CRC failed: Humidity");
        ha_humidity_->SetValue(0, 0);
    }
    else
    {
        ha_humidity_->SetValue(device_.toPercent(), 0);
    }

    if (device_.passTcrc == false)
    {
        logger_.LogError("Sensor CRC failed: Temperature");
        ha_temperature_->SetValue(0);
    }
    else
    {
        ha_temperature_->SetValue(device_.toDegC());
    }
    return true;
}

void Shtc3::InternalPowerDown()
{
}
