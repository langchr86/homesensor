#include <chrono>

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>

#include <PubSubClient.h>

#include "sensors/adc.h"
#include "sensors/shtc3.h"

static constexpr size_t kWireSpeedHz = 100000;
static constexpr size_t kSerialSpeedBaud = 115200;

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

void Reboot(HardwareSerial *serial)
{
  const auto kWaitBeforeReboot = std::chrono::seconds(3);
  if (serial != nullptr)
  {
    serial->print("Restarting in ");
    serial->print(static_cast<int>(kWaitBeforeReboot.count()));
    serial->println(" seconds");
  }
  FlashErrorLED(kWaitBeforeReboot);
  ESP.restart();
}

bool InitSerial(HardwareSerial *serial)
{
  serial->begin(kSerialSpeedBaud);
  delay(1000);
  serial->println("Serial console setup finished");
  return true;
}

bool InitWire(HardwareSerial *serial, TwoWire *wire)
{
  if (wire->begin() == false)
  {
    serial->println("Failed to setup i2C");
    return false;
  }
  wire->setClock(kWireSpeedHz);

  serial->println("i2C setup finished");
  return true;
}

bool InitWifi(HardwareSerial *serial, WiFiClass *wifi)
{
  if (wifi->mode(WIFI_STA) == false)
  {
    serial->println("Failed to setup WIFI mode");
    return false;
  }
  if (wifi->config(kSensorIp, kGatewayIp, kSubnetMask) == false)
  {
    serial->println("Failed to setup WIFI IP config");
    return false;
  }

  serial->println("Basic WIFI setup finished");
  return true;
}

bool ConnectWifiAndMqtt(HardwareSerial *serial, WiFiClass *wifi, PubSubClient *mqtt)
{
  serial->println("Try to connect to WIFI");
  wifi->begin(kWifiSsid, kWifiPassword);
  if (wifi->waitForConnectResult() != WL_CONNECTED)
  {
    serial->println("WIFI connection failed");
    return false;
  }

  if (wifi->setHostname(kSensorId) == false)
  {
    serial->println("Failed to setup WIFI hostname");
    return false;
  }

  serial->println("Try to connect to MQTT broker");
  if (mqtt->connect(kSensorId, kMqttUser, kMqttPassword) == false)
  {
    serial->print("MQTT connection failed! Error code = ");
    serial->println(mqtt->getWriteError());
    return false;
  }

  return true;
}

void DisconnectWifiAndMqtt(HardwareSerial *serial, WiFiClass *wifi, PubSubClient *mqtt)
{
  mqtt->disconnect();
  if (wifi->disconnect() == false)
  {
    serial->print("Failed to disconnect WIFI");
  }
}

void setup()
{
  DisableLED();

  auto *serial = &Serial;
  if (InitSerial(serial) == false)
  {
    Reboot(nullptr);
  }

  auto *wire = &Wire;
  if (InitWire(serial, wire) == false)
  {
    Reboot(serial);
  }

  auto *wifi = &WiFi;
  if (InitWifi(serial, wifi) == false)
  {
    Reboot(serial);
  }

  WiFiClient wifiClient;
  PubSubClient mqttClient(kHomeAssistantIp, kMqttPort, wifiClient);
  auto *mqtt = &mqttClient;
  mqtt->setBufferSize(kMqttMaxMessageSize);
  serial->println("Basic MQTT setup finished");

  ADC adc_instance(serial, A0);
  auto *adc = &adc_instance;
  adc->SetBitWidth(10);
  serial->println("ADC setup finished");

  const std::chrono::seconds expire_timeout(3 * kReadOutInterval);
  Shtc3 sensor(serial, adc, wire, mqtt, kSensorName, kSensorId, expire_timeout);
  if (sensor.InitHardware() == false)
  {
    serial->println("Failed to initialize sensor hardware");
    Reboot(serial);
  }
  serial->println("Sensor setup finished");

  if (ConnectWifiAndMqtt(serial, wifi, mqtt) == false)
  {
    serial->println("Failed to connect for initial HA config messages");
    Reboot(serial);
  }

  if (sensor.SendHomeassistantConfig() == false)
  {
    serial->println("Failed to send initial HA config messages");
    Reboot(serial);
  }

  DisconnectWifiAndMqtt(serial, wifi, mqtt);
  serial->println("Initial HA config messages sent");

  serial->println("Finished basic setup. Starting readout interval");

  // === readout loop ====================================================================
  while (true)
  {
    if (ConnectWifiAndMqtt(serial, wifi, mqtt) == false)
    {
      FlashErrorLED();
      continue;
    }

    if (sensor.Loop() == false)
    {
      FlashErrorLED();
      DisconnectWifiAndMqtt(serial, wifi, mqtt);
      continue;
    }

    DisconnectWifiAndMqtt(serial, wifi, mqtt);

    serial->println("going to deep sleep");
    esp_deep_sleep(std::chrono::duration_cast<std::chrono::microseconds>(kReadOutInterval).count());
  }
}

// we use only setup() function
void loop() {}

// TODO(clang)
// auto co2_sensor = std::make_shared<Sensor>("CO2", "co2", SensorDeviceClass::kNone, "ppm", "mdi:molecule-co2");
// co2_sensor->SetExpireTimeout(expire_timeout);
// device.AddSensor(co2_sensor);
