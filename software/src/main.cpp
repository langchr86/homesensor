#include <Arduino.h>
#include <Wire.h>

#include "communication/connection.h"
#include "infrastructure/device_factory.h"
#include "sensors/adc.h"
#include "utils/led.h"
#include "utils/logger.h"
#include "utils/power.h"

#include "config.h"

std::shared_ptr<SensorInterface> sensor = nullptr;

RTC_DATA_ATTR size_t boot_count = 0;
RTC_DATA_ATTR size_t failed_boot_count = 0;
RTC_DATA_ATTR size_t failed_consecutive_boots = 0;
RTC_DATA_ATTR std::chrono::seconds readout_interval = kDefaultReadoutInterval;
RTC_DATA_ATTR std::chrono::seconds max_readout_interval = readout_interval;

bool InitWire(Logger *logger, TwoWire *wire)
{
  if (wire->begin() == false)
  {
    logger->LogError("Failed to setup i2C");
    return false;
  }
  wire->setClock(kWireSpeedHz);

  return true;
}

void Sleep(Power *power, Logger *logger)
{
  if (sensor->LowPower())
  {
    logger->LogWarning("Force kMaxReadOutInterval because of low power");
    power->DeepSleepNow(kMaxReadoutInterval);
  }
  else
  {
    power->DeepSleepNow(readout_interval);
  }
}

void ErrorHappened(Connection *connection, Power *power, Logger *logger)
{
  if (connection != nullptr)
  {
    connection->Disconnect();
  }

  Led::FlashFor();
  failed_boot_count++;
  failed_consecutive_boots++;

  if (failed_consecutive_boots > 3)
  {
    readout_interval *= 2;
    failed_consecutive_boots = 0;

    if (readout_interval > kMaxReadoutInterval)
    {
      readout_interval = kMaxReadoutInterval;
    }

    if (max_readout_interval < readout_interval)
    {
      max_readout_interval = readout_interval;
    }

    logger->LogInfo("Increased read out interval to %u seconds", static_cast<int>(readout_interval.count()));
  }

  Sleep(power, logger);
}

void EndWithNoError(Connection *connection, Power *power, Logger *logger)
{
  failed_consecutive_boots = 0;
  readout_interval = kDefaultReadoutInterval;
  connection->Disconnect();
  Sleep(power, logger);
}

void setup()
{
  boot_count++;

  Led::Disable();
  Power power;

  Logger logger("Main");
  logger.SetMaxLevel(LOG_INFO);
  logger.LogInfo("boot_count=%u", boot_count);
  logger.LogInfo("failed_boot_count=%u", failed_boot_count);

  auto *wire = &Wire;
  if (InitWire(&logger, wire) == false)
  {
    logger.LogError("Failed to initialize Wire connection");
    ErrorHappened(nullptr, &power, &logger);
  }
  logger.LogDebug("Success: InitWire");

  ADC adc(A0);
  adc.SetBitWidth(9);

  Connection connection(kHomeAssistantIp, kMqttPort, kGatewayIp, kSubnetMask, kMqttMaxMessageSize);

  const std::chrono::seconds expire_timeout(3 * kDefaultReadoutInterval);
  DeviceFactory factory(readout_interval, expire_timeout);

  sensor = factory.CreateDevice(config[kDeviceConfigIndex], &adc, wire, &connection, &power);

  if (sensor == nullptr)
  {
    logger.LogError("Failed to create sensor device");
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: factory.CreateDevice");

  if (sensor->SensorReadLoop() == false)
  {
    logger.LogError("Sensor read out loop failed");
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: sensor.SensorReadLoop");

  sensor->SetDebugInfos(boot_count, failed_boot_count, max_readout_interval);

  if (connection.Init() == false)
  {
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: connection.Init");

  if (connection.Connect(config[kDeviceConfigIndex].unique_id.c_str(), kWifiSsid, kWifiPassword, kMqttUser, kMqttPassword) == false)
  {
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: connection.Connect");

  if (boot_count == 1 || failed_consecutive_boots > 0) // only send HA config at first boot or after failed boot
  {
    if (sensor->SendHomeassistantConfig() == false)
    {
      ErrorHappened(&connection, &power, &logger);
    }
  }

  if (sensor->SendHomeassistantState() == false)
  {
    ErrorHappened(&connection, &power, &logger);
  }

  EndWithNoError(&connection, &power, &logger);
}

// we use only setup() function
void loop() {}
