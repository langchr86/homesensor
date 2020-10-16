#include <chrono>

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>

#include <PubSubClient.h>

#include "sensors/adc.h"
#include "sensors/shtc3.h"
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

void DisableLED()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void FlashErrorLED(std::chrono::milliseconds duration = std::chrono::milliseconds(3000))
{
  constexpr auto kLedIntervalDuration = std::chrono::milliseconds(250);
  assert(duration >= 2 * kLedIntervalDuration);
  const size_t iteration_count = duration / (2 * kLedIntervalDuration);

  pinMode(LED_BUILTIN, OUTPUT);
  for (size_t i = 0; i < iteration_count; ++i)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(kLedIntervalDuration.count());
    digitalWrite(LED_BUILTIN, LOW);
    delay(kLedIntervalDuration.count());
  }
}

void Reboot(Logger *logger)
{
  const auto kWaitBeforeReboot = std::chrono::seconds(3);
  logger->LogWarning("Restarting in %u seconds", static_cast<int>(kWaitBeforeReboot.count()));
  FlashErrorLED(kWaitBeforeReboot);
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

bool InitWifi(Logger *logger, WiFiClass *wifi)
{
  if (wifi->mode(WIFI_STA) == false)
  {
    logger->LogError("Failed to setup WIFI mode");
    return false;
  }
  if (wifi->config(kSensorIp, kGatewayIp, kSubnetMask) == false)
  {
    logger->LogError("Failed to setup WIFI IP config");
    return false;
  }

  logger->LogInfo("Basic WIFI setup finished");
  return true;
}

bool ConnectWifiAndMqtt(Logger *logger, WiFiClass *wifi, PubSubClient *mqtt)
{
  logger->LogInfo("Try to connect to WIFI");
  wifi->begin(kWifiSsid, kWifiPassword);
  if (wifi->waitForConnectResult() != WL_CONNECTED)
  {
    logger->LogError("WIFI connection failed");
    return false;
  }

  if (wifi->setHostname(kSensorId) == false)
  {
    logger->LogError("Failed to setup WIFI hostname");
    return false;
  }

  logger->LogInfo("Try to connect to MQTT broker");
  if (mqtt->connect(kSensorId, kMqttUser, kMqttPassword) == false)
  {
    logger->LogError("MQTT connection failed! Error code = %i", mqtt->getWriteError());
    return false;
  }

  return true;
}

void DisconnectWifiAndMqtt(Logger *logger, WiFiClass *wifi, PubSubClient *mqtt)
{
  mqtt->disconnect();
  if (wifi->disconnect() == false)
  {
    logger->LogError("Failed to disconnect WIFI");
  }
}

void DeepSleepNow(Logger *logger, const std::chrono::seconds &duration)
{
  logger->LogInfo("going to deep sleep");
  esp_deep_sleep(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}

void setup()
{
  DisableLED();

  Logger logger("Main");

  auto *wire = &Wire;
  if (InitWire(&logger, wire) == false)
  {
    Reboot(&logger);
  }

  auto *wifi = &WiFi;
  if (InitWifi(&logger, wifi) == false)
  {
    Reboot(&logger);
  }

  WiFiClient wifiClient;
  PubSubClient mqttClient(kHomeAssistantIp, kMqttPort, wifiClient);
  auto *mqtt = &mqttClient;
  mqtt->setBufferSize(kMqttMaxMessageSize);
  logger.LogInfo("Basic MQTT setup finished");

  ADC adc_instance(A0);
  auto *adc = &adc_instance;
  adc->SetBitWidth(10);
  logger.LogInfo("ADC setup finished");

  const std::chrono::seconds expire_timeout(3 * kReadOutInterval);
  Shtc3 sensor(adc, wire, mqtt, kSensorName, kSensorId, expire_timeout);
  if (sensor.InitHardware() == false)
  {
    logger.LogError("Failed to initialize sensor hardware");
    Reboot(&logger);
  }
  logger.LogInfo("Sensor setup finished");

  if (ConnectWifiAndMqtt(&logger, wifi, mqtt) == false)
  {
    logger.LogError("Failed to connect for initial HA config messages");
    Reboot(&logger);
  }

  if (sensor.SendHomeassistantConfig() == false)
  {
    logger.LogError("Failed to send initial HA config messages");
    Reboot(&logger);
  }

  DisconnectWifiAndMqtt(&logger, wifi, mqtt);
  logger.LogInfo("Initial HA config messages sent");

  logger.LogInfo("Finished basic setup. Starting readout interval");

  // === readout loop ====================================================================
  while (true)
  {
    if (ConnectWifiAndMqtt(&logger, wifi, mqtt) == false)
    {
      FlashErrorLED();
      continue;
    }

    if (sensor.Loop() == false)
    {
      FlashErrorLED();
      DisconnectWifiAndMqtt(&logger, wifi, mqtt);
      continue;
    }

    DisconnectWifiAndMqtt(&logger, wifi, mqtt);

    DeepSleepNow(&logger, kReadOutInterval);
  }
}

// we use only setup() function
void loop() {}

// TODO(clang)
// auto co2_sensor = std::make_shared<Sensor>("CO2", "co2", SensorDeviceClass::kNone, "ppm", "mdi:molecule-co2");
// co2_sensor->SetExpireTimeout(expire_timeout);
// device.AddSensor(co2_sensor);
