#include <chrono>

#include <Arduino.h>
#include <Wire.h>

#include "communication/connection.h"
#include "infrastructure/device_factory.h"
#include "sensors/adc.h"
#include "utils/led.h"
#include "utils/logger.h"
#include "utils/power.h"

static constexpr size_t kWireSpeedHz = 100000;

static constexpr char kSensorName[] = "Balkon";
static constexpr char kSensorId[] = "balkon";
static constexpr std::chrono::seconds kDefaultReadOutInterval = std::chrono::minutes(1);
static constexpr std::chrono::seconds kMaxReadOutInterval = std::chrono::minutes(30);
static_assert(kMaxReadOutInterval > kDefaultReadOutInterval, "kMaxReadOutInterval needs to be bigger then kDefaultReadOutInterval");

static constexpr char kWifiSsid[] = "";
static constexpr char kWifiPassword[] = "";

static const IPAddress kHomeAssistantIp(192, 168, 0, 25);
static const IPAddress kSensorIp(192, 168, 0, 11);
static const IPAddress kGatewayIp(192, 168, 0, 1);
static const IPAddress kSubnetMask(255, 255, 255, 0);

static constexpr char kMqttUser[] = "";
static constexpr char kMqttPassword[] = "";
static constexpr uint16_t kMqttPort = 1883;
static constexpr size_t kMqttMaxMessageSize = 512;

RTC_DATA_ATTR size_t boot_count = 0;
RTC_DATA_ATTR size_t failed_boot_count = 0;
RTC_DATA_ATTR size_t failed_consecutive_boots = 0;
RTC_DATA_ATTR std::chrono::seconds read_out_interval = kDefaultReadOutInterval;

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
    read_out_interval *= 2;
    failed_consecutive_boots = 0;

    if (read_out_interval > kMaxReadOutInterval)
    {
      read_out_interval = kMaxReadOutInterval;
    }

    logger->LogInfo("Increased read out interval to %u seconds", static_cast<int>(read_out_interval.count()));
  }

  power->DeepSleepNow(read_out_interval);
}

void EndWithNoError(Connection *connection, Power *power)
{
  failed_consecutive_boots = 0;
  read_out_interval = kDefaultReadOutInterval;
  connection->Disconnect();
  power->DeepSleepNow(read_out_interval);
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
  adc.SetBitWidth(10);

  Connection connection(kHomeAssistantIp, kMqttPort, kGatewayIp, kSubnetMask, kMqttMaxMessageSize);

  const std::chrono::seconds expire_timeout(3 * kDefaultReadOutInterval);
  DeviceFactory factory(read_out_interval, expire_timeout);

  DeviceConfig config;
  config.type = DeviceType::kShtc3;
  config.name = kSensorName;
  config.unique_id = kSensorId;
  config.ip_address = kSensorIp;
  const auto sensor = factory.CreateDevice(config, &adc, wire, &connection);

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

  if (connection.Init() == false)
  {
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: connection.Init");

  if (connection.Connect(kSensorId, kWifiSsid, kWifiPassword, kMqttUser, kMqttPassword) == false)
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

  EndWithNoError(&connection, &power);
}

// we use only setup() function
void loop() {}
