#include "shtc3.h"

Shtc3::Shtc3(ADC *adc, TwoWire *wire, Connection *connection, Power *power, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout)
    : SensorBase(adc, connection, power, readable_name, unique_id, "SHTC3"), wire_(wire)
{
    ha_temperature_ = std::make_shared<Sensor>("Temperatur", "temperature", SensorDeviceClass::kTemperature, "°C");
    ha_temperature_->SetExpireTimeout(expire_timeout);
    ha_temperature_->SetCalibration(4.1, 25.1, 3.2, 24.4);
    ha_device_->AddSensor(ha_temperature_);

    ha_humidity_ = std::make_shared<Sensor>("Feuchtigkeit", "humidity", SensorDeviceClass::kHumidity, "%");
    ha_humidity_->SetExpireTimeout(expire_timeout);
    ha_device_->AddSensor(ha_humidity_);
}

bool Shtc3::HardwareInitialization(const std::chrono::seconds &readout_interval)
{
    return true;
}

bool Shtc3::ForceCalibrationNow()
{
    return true;
}

bool Shtc3::InternalPowerUp()
{
    wire_->setClockStretchLimit(200000);

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

    if (device_.wake(true) != SHTC3_Status_Nominal)
    {
        logger_.LogError("Failed to wake up sensor");
        return false;
    }

    return true;
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
        return false;
    }
    if (device_.passTcrc == false)
    {
        logger_.LogError("Sensor CRC failed: Temperature");
        return false;
    }

    ha_humidity_->SetValue(device_.toPercent());
    ha_temperature_->SetValue(device_.toDegC());
    return true;
}

void Shtc3::InternalPowerDown()
{
}
