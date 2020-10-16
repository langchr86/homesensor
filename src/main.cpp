#include <chrono>

#include <Arduino.h>
#include <Wire.h>

#include "communication/connection.h"
#include "sensors/adc.h"
#include "sensors/shtc3.h"
#include "utils/led.h"
#include "utils/logger.h"

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


void Reboot(Logger *logger)
{
  const auto kWaitBeforeReboot = std::chrono::seconds(3);
  logger->LogWarning("Restarting in %u seconds", static_cast<int>(kWaitBeforeReboot.count()));
  Led::FlashFor(kWaitBeforeReboot);
  ESP.restart();
}

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

void SetPowerOption(Logger *logger, esp_sleep_pd_domain_t domain, esp_sleep_pd_option_t option)
{
  const auto result = esp_sleep_pd_config(domain, option);
  if (result != ESP_OK)
  {
    logger->LogError("failed to set power option: domain=%i option=%i", domain, option);
  }
}

void DeepSleepNow(Logger *logger, const std::chrono::seconds &duration)
{
  SetPowerOption(logger, ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  logger->LogInfo("going to deep sleep");
  esp_deep_sleep(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}

void setup()
{
  Led::Disable();

  Logger logger("Main");

  auto *wire = &Wire;
  if (InitWire(&logger, wire) == false)
  {
    Reboot(&logger);
  }

  Connection connection(kHomeAssistantIp, kMqttPort);
  if (connection.Init(kSensorIp, kGatewayIp, kSubnetMask, kMqttMaxMessageSize) == false)
  {
    logger.LogError("Failed to initialize connection");
    Reboot(&logger);
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
    Reboot(&logger);
  }
  logger.LogInfo("Sensor setup finished");

  if (connection.Connect(kSensorId, kWifiSsid, kWifiPassword, kMqttUser, kMqttPassword) == false)
  {
    logger.LogError("Failed to connect for initial HA config messages");
    Reboot(&logger);
  }

  if (sensor.SendHomeassistantConfig() == false)
  {
    logger.LogError("Failed to send initial HA config messages");
    Reboot(&logger);
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

    DeepSleepNow(&logger, kReadOutInterval);
  }
}

// we use only setup() function
void loop() {}

// TODO(clang)
// auto co2_sensor = std::make_shared<Sensor>("CO2", "co2", SensorDeviceClass::kNone, "ppm", "mdi:molecule-co2");
// co2_sensor->SetExpireTimeout(expire_timeout);
// device.AddSensor(co2_sensor);
