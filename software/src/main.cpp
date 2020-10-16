#include <chrono>

#include <Arduino.h>
#include <Wire.h>

#include "communication/connection.h"
#include "sensors/adc.h"
#include "sensors/scd30.h"
#include "sensors/shtc3.h"
#include "utils/led.h"
#include "utils/logger.h"
#include "utils/power.h"

static constexpr size_t kWireSpeedHz = 100000;

static constexpr char kSensorName[] = "Balkon";
static constexpr char kSensorId[] = "balkon";
static constexpr auto kDefaultReadOutInterval = std::chrono::seconds(10);
static constexpr auto kMaxReadOutInterval = kDefaultReadOutInterval * 10;

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
  failed_consecutive_boots++;

  if (failed_consecutive_boots > 3)
  {
    read_out_interval *= 2;
    failed_consecutive_boots = 0;

    if (read_out_interval > kMaxReadOutInterval)
    {
      read_out_interval = kMaxReadOutInterval;
    }

    logger->LogInfo("Increased read out interval to %u seconds", read_out_interval.count());
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

  auto *wire = &Wire;
  if (InitWire(&logger, wire) == false)
  {
    logger.LogError("Failed to initialize Wire connection");
    ErrorHappened(nullptr, &power, &logger);
  }
  logger.LogDebug("Success: InitWire");

  Connection connection(kHomeAssistantIp, kMqttPort);
  if (connection.Init(kSensorIp, kGatewayIp, kSubnetMask, kMqttMaxMessageSize) == false)
  {
    logger.LogError("Failed to initialize Wifi/Mqtt connection");
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: connection.Init");

  ADC adc(A0);
  adc.SetBitWidth(10);

  const std::chrono::seconds expire_timeout(3 * kDefaultReadOutInterval);
  Shtc3 sensor(&adc, wire, &connection, kSensorName, kSensorId, expire_timeout);
  if (sensor.InitHardware() == false)
  {
    logger.LogError("Failed to initialize sensor hardware");
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: sensor.InitHardware");

  if (sensor.SensorReadLoop() == false)
  {
    logger.LogError("Sensor read out loop failed");
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: sensor.SensorReadLoop");

  if (connection.Connect(kSensorId, kWifiSsid, kWifiPassword, kMqttUser, kMqttPassword) == false)
  {
    logger.LogError("Failed to connect for initial HA config messages");
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: connection.Connect");

  if (boot_count == 1) // only send HA config at first boot
  {
    if (sensor.SendHomeassistantConfig() == false)
    {
      ErrorHappened(&connection, &power, &logger);
    }
  }

  if (sensor.SendHomeassistantState() == false)
  {
    ErrorHappened(&connection, &power, &logger);
  }

  EndWithNoError(&connection, &power);
}

// we use only setup() function
void loop() {}
