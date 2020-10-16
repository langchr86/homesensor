#include <chrono>

#include <Arduino.h>
#include <Wire.h>

#include "communication/connection.h"
#include "sensors/adc.h"
#include "sensors/shtc3.h"
#include "utils/led.h"
#include "utils/logger.h"
#include "utils/power.h"

static constexpr size_t kWireSpeedHz = 100000;

static constexpr char kSensorName[] = "Balkon";
static constexpr char kSensorId[] = "balkon";
static constexpr auto kReadOutInterval = std::chrono::seconds(10);

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


bool InitWire(Logger *logger, TwoWire *wire)
{
  if (wire->begin() == false)
  {
    logger->LogError("Failed to setup i2C");
    return false;
  }
  wire->setClock(kWireSpeedHz);

  logger->LogInfo("i2C setup finished");
  return true;
}

void setup()
{
  Led::Disable();
  Power power;

  Logger logger("Main");

  auto *wire = &Wire;
  if (InitWire(&logger, wire) == false)
  {
    power.Reboot();
  }

  Connection connection(kHomeAssistantIp, kMqttPort);
  if (connection.Init(kSensorIp, kGatewayIp, kSubnetMask, kMqttMaxMessageSize) == false)
  {
    logger.LogError("Failed to initialize connection");
    power.Reboot();
  }

  ADC adc_instance(A0);
  auto *adc = &adc_instance;
  adc->SetBitWidth(10);
  logger.LogInfo("ADC setup finished");

  const std::chrono::seconds expire_timeout(3 * kReadOutInterval);
  Shtc3 sensor(adc, wire, &connection, kSensorName, kSensorId, expire_timeout);
  if (sensor.InitHardware() == false)
  {
    logger.LogError("Failed to initialize sensor hardware");
    power.Reboot();
  }
  logger.LogInfo("Sensor setup finished");

  if (connection.Connect(kSensorId, kWifiSsid, kWifiPassword, kMqttUser, kMqttPassword) == false)
  {
    logger.LogError("Failed to connect for initial HA config messages");
    power.Reboot();
  }

  if (sensor.SendHomeassistantConfig() == false)
  {
    logger.LogError("Failed to send initial HA config messages");
    power.Reboot();
  }

  connection.Disconnect();
  logger.LogInfo("Initial HA config messages sent");

  logger.LogInfo("Finished basic setup. Starting readout interval");

  // === readout loop ====================================================================
  while (true)
  {
    if (connection.Connect(kSensorId, kWifiSsid, kWifiPassword, kMqttUser, kMqttPassword) == false)
    {
      Led::FlashFor();
      continue;
    }

    if (sensor.Loop() == false)
    {
      Led::FlashFor();
      connection.Disconnect();
      continue;
    }

    connection.Disconnect();

    power.DeepSleepNow(kReadOutInterval);
  }
}

// we use only setup() function
void loop() {}

// TODO(clang)
// auto co2_sensor = std::make_shared<Sensor>("CO2", "co2", SensorDeviceClass::kNone, "ppm", "mdi:molecule-co2");
// co2_sensor->SetExpireTimeout(expire_timeout);
// device.AddSensor(co2_sensor);
