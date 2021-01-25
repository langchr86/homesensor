#include "scd30.h"

Scd30::Scd30(ADC *adc, TwoWire *wire, Connection *connection, Power *power, const char *readable_name, const char *unique_id, const std::chrono::seconds &expire_timeout)
    : SensorBase(adc, connection, power, readable_name, unique_id, "SCD30"), wire_(wire)
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

bool Scd30::HardwareInitialization(const std::chrono::seconds &readout_interval)
{
    readout_interval_ = readout_interval;

    power_->LightSleepNow(kHardwareStartupDuration);
    logger_.LogDebug("SCD30 should now be booted and should respond");

    if (Connect() == false)
    {
        return false;
    }

    if (device_.StopMeasurement() == false)
    {
        logger_.LogError("Failed stop measuring");
        return false;
    }
    logger_.LogDebug("Stopped measurement");

    if (device_.setAutoSelfCalibration(true) == false)
    {
        logger_.LogError("Failed to disable ASC");
        return false;
    }

    // temperature calibration is done by Sensor::SetCalibration
    if (device_.setTemperatureOffset(1.2f) == false)
    {
        logger_.LogError("Failed to set temperature offset");
        return false;
    }

    // set minimal readout interval for initial measurement
    if (device_.setMeasurementInterval(kMinReadoutInterval.count()) == false)
    {
        logger_.LogError("Failed to set minimal measurement interval");
        return false;
    }

    // start measuring without ambient pressure compensation
    if (device_.beginMeasuring(0) == false)
    {
        logger_.LogError("Failed start continuous measuring");
        return false;
    }
    logger_.LogDebug("Started for initial measurement");

    if (WaitForDataAvailable(kMinReadoutInterval * 2) == false)
    {
        return false;
    }

    // set real measurement interval
    if (device_.setMeasurementInterval(readout_interval.count()) == false)
    {
        logger_.LogError("Failed to set real readout interval");
        return false;
    }
    logger_.LogDebug("Changed to real readout interval");

    return true;
}

bool Scd30::InternalPowerUp()
{
    return Connect();
}

bool Scd30::InternalSensorMeasurement()
{
    return WaitForDataAvailable(readout_interval_);
}

void Scd30::InternalPowerSave()
{
}

bool Scd30::InternalSensorRead()
{
    ha_humidity_->SetValue(device_.getHumidity(), 0);
    ha_temperature_->SetValue(device_.getTemperature());
    ha_co2_->SetValue(device_.getCO2(), 0);

    return true;
}

void Scd30::InternalPowerDown()
{
}

bool Scd30::Connect()
{
    // this sets internal used wire instance, and if needed clock stretching but does not start anything
    if (device_.begin(*wire_, false) == false)
    {
        logger_.LogError("Failed to setup wire connection");
        return false;
    }

    return true;
}

bool Scd30::WaitForDataAvailable(const std::chrono::milliseconds &timeout)
{
    const auto data_available = power_->WaitUntil(timeout, std::chrono::milliseconds(100), [this]() {
        return device_.dataAvailable();
    });
    if (data_available == false)
    {
        logger_.LogError("Failed read out measuring data");
        return false;
    }
    return true;
}

bool Scd30::ForceRecalibrationvalue()
{
    return device_.setForceRecalibration(kFreshAirReferencePpm);
}
