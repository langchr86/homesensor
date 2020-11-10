#include "infrastructure/device_factory.h"

#include "sensors/scd30.h"
#include "sensors/shtc3.h"

DeviceFactory::DeviceFactory(const std::chrono::seconds &readout_interval, const std::chrono::seconds &expire_timeout)
    : logger_("DeviceFactory"), readout_interval_(readout_interval), expire_timeout_(expire_timeout)
{
}

std::shared_ptr<SensorInterface> DeviceFactory::CreateDevice(const DeviceConfig &config, ADC *adc, TwoWire *wire, Connection *connection)
{
    std::shared_ptr<SensorInterface> sensor;
    switch (config.type)
    {
    case DeviceType::kShtc3:
        sensor = std::make_shared<Shtc3>(adc, wire, connection, config.name.c_str(), config.unique_id.c_str(), expire_timeout_);
        break;
    case DeviceType::kScd30:
        sensor = std::make_shared<Scd30>(adc, wire, connection, config.name.c_str(), config.unique_id.c_str(), expire_timeout_);
        break;
    }

    if (connection->Init(config.ip_address) == false)
    {
        logger_.LogError("Failed to initialize Wifi/Mqtt connection");
        return nullptr;
    }
    logger_.LogDebug("Success: connection.Init");

    return sensor;
}
