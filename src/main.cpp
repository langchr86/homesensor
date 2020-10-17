#include <memory>

#include <Arduino.h>
#include <WiFi.h>

#define MQTT_MAX_PACKET_SIZE 256 // needs to be directly set in the library files
#include <ArduinoMqttClient.h>

#include "sensor_device.h"

static const std::string kWifiSsid = "";
static const std::string kWifiPassword = "";

static const std::string kHomeAssistantIp = "192.168.0.25";
static const uint16_t kMqttPort = 1883;
static const std::string kMqttUser = "";
static const std::string kMqttPassword = "";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

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

  IPAddress ip;
  ip.fromString("192.168.0.11");
  IPAddress gateway;
  gateway.fromString("192.168.0.1");
  IPAddress subnet;
  subnet.fromString("255.255.255.0");

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(device.GetUniqueId().c_str());
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(kWifiSsid.c_str(), kWifiPassword.c_str());
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    blinkError(5);
    ESP.restart();
    Serial.println("restarting");
  }

  IPAddress ha_ip;
  ha_ip.fromString(kHomeAssistantIp.c_str());

  mqttClient.setId(device.GetUniqueId().c_str());
  mqttClient.setUsernamePassword(kMqttUser.c_str(), kMqttPassword.c_str());

  if (!mqttClient.connect(ha_ip, kMqttPort))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
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

  mqttClient.poll();

  for (const auto &message : device.GetAllConfigMessages())
  {
    mqttClient.beginMessage(message.GetTopic(), false);
    mqttClient.print(message.GetPayload());
    mqttClient.endMessage();

    Serial.print("Sent config message to: ");
    Serial.println(message.GetTopic());
    Serial.println(message.GetPayload());
    Serial.println("-------------------");
  }

  // TODO(clang): temperature_sensor->SetValue(9.9);

  const auto state_message = device.GetStateMessage();
  mqttClient.beginMessage(state_message.GetTopic(), false);
  mqttClient.print(state_message.GetPayload());
  mqttClient.endMessage();

  Serial.println("Sent state message to: ");
  Serial.println(state_message.GetTopic());
  Serial.println(state_message.GetPayload());
  Serial.println("-------------------");

  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
