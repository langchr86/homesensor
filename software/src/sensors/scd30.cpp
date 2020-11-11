#include "scd30.h"

Scd30::Scd30(ADC *adc, TwoWire *wire, Connection *connection, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout)
    : SensorBase(adc, connection, readable_name, unique_id, expire_timeout, "SCD30"), wire_(wire)
{
    ha_temperature_ = std::make_shared<Sensor>("Temperatur", "temperature", SensorDeviceClass::kTemperature, "°C");
    ha_temperature_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_temperature_);

    ha_humidity_ = std::make_shared<Sensor>("Feuchtigkeit", "humidity", SensorDeviceClass::kHumidity, "%");
    ha_humidity_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_humidity_);

    ha_co2_ = std::make_shared<Sensor>("CO2", "co2", SensorDeviceClass::kNone, "ppm", "mdi:molecule-co2");
    ha_co2_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_co2_);
}

bool Scd30::InternalPowerUp()
{
    if (device_.begin(*wire_) == false)
    {
        logger_.LogError("Failed to setup wire connection to sensor");
        return false;
    }

    return true;
}

bool Scd30::InternalSensorMeasurement()
{
    if (device_.dataAvailable() == false)
    {
        logger_.LogError("No data available");
        return false;
    }

    ha_humidity_->SetValue(device_.getHumidity(), 0);
    ha_temperature_->SetValue(device_.getTemperature());
    ha_co2_->SetValue(device_.getCO2());

    return true;
}

void Scd30::InternalPowerSave()
{
}

bool Scd30::InternalSensorRead()
{
    return true;
}

void Scd30::InternalPowerDown()
{
    // TODO(clang): use real readout interval
    if (device_.setMeasurementInterval(30) == false)
    {
        logger_.LogError("Failed to set measurement interval");
    }
}
