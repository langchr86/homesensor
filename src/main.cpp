#include <memory>

#include <Arduino.h>
#include <WiFi.h>

#include <PubSubClient.h>

#include "sensor_device.h"

static constexpr char kWifiSsid[] = "";
static constexpr char kWifiPassword[] = "";

static const IPAddress kHomeAssistantIp(192, 168, 0, 25);
static const uint16_t kMqttPort = 1883;
static constexpr char kMqttUser[] = "";
static constexpr char kMqttPassword[] = "";
static constexpr size_t kMqttMaxMessageSize = 512;

WiFiClient wifiClient;
PubSubClient mqttClient(kHomeAssistantIp, kMqttPort, wifiClient);

SensorDevice device("Balkon", "balkon", "firebeetle32", "espressif");

void blinkError(size_t seconds)
{
  for (size_t i = 0; i < seconds * 2; ++i)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Booting");

  pinMode(LED_BUILTIN, OUTPUT);

  const IPAddress ip(192, 168, 0, 11);
  const IPAddress gateway(192, 168, 0, 1);
  const IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(device.GetUniqueId().c_str());
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(kWifiSsid, kWifiPassword);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    blinkError(5);
    ESP.restart();
    Serial.println("restarting");
  }

  mqttClient.setBufferSize(kMqttMaxMessageSize);

  if (!mqttClient.connect(device.GetUniqueId().c_str(), kMqttUser, kMqttPassword))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.getWriteError());
    blinkError(5);
    ESP.restart();
    Serial.println("restarting");
  }

  const std::chrono::seconds expire_timeout(3);
  auto temperature_sensor = std::make_shared<Sensor>("Temperatur", "temperature", SensorDeviceClass::kTemperature, "°C");
  temperature_sensor->SetExpireTimeout(expire_timeout);
  temperature_sensor->SetValue(23.3),
      device.AddSensor(temperature_sensor);

  auto humidity_sensor = std::make_shared<Sensor>("Feuchtigkeit", "humidity", SensorDeviceClass::kHumidity, "%");
  humidity_sensor->SetExpireTimeout(expire_timeout);
  humidity_sensor->SetValue(55.5);
  device.AddSensor(humidity_sensor);

  auto co2_sensor = std::make_shared<Sensor>("CO2", "co2", SensorDeviceClass::kNone, "ppm", "mdi:molecule-co2");
  co2_sensor->SetExpireTimeout(expire_timeout);
  co2_sensor->SetValue(444, 0);
  device.AddSensor(co2_sensor);
}

void loop()
{
  Serial.println("I am working");

  for (const auto &message : device.GetAllConfigMessages())
  {
    const auto result = mqttClient.publish(message.GetTopic(), message.GetPayload());

    Serial.print("Sent config message to: ");
    Serial.println(message.GetTopic());
    Serial.println(message.GetPayload());
    if (result == false)
    {
      Serial.println("failed to send!");
    }
    Serial.println("-------------------");
  }

  // TODO(clang): temperature_sensor->SetValue(9.9);

  const auto state_message = device.GetStateMessage();
  mqttClient.publish(state_message.GetTopic(), state_message.GetPayload());

  Serial.println("Sent state message to: ");
  Serial.println(state_message.GetTopic());
  Serial.println(state_message.GetPayload());
  Serial.println("-------------------");

  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
