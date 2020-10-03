#include <string>

#include <Arduino.h>
#include <WiFi.h>

#define MQTT_MAX_PACKET_SIZE 256  // needs to be directly set in the library files
#include <ArduinoMqttClient.h>

#include "auto_discovery.h"

static const std::string kRoomName = "balkon";
static const std::string kWifiSsid = "";
static const std::string kWifiPassword = "";

static const std::string kHomeAssistantIp = "192.168.0.25";
static const uint16_t kMqttPort = 1883;
static const std::string kMqttUser = "";
static const std::string kMqttPassword = "";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

AutoDiscovery temperature_sensor;

void blinkError(size_t seconds) {
  for (size_t i = 0; i < seconds * 2; ++i) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}

void setup() {
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
  WiFi.setHostname(kRoomName.c_str());
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(kWifiSsid.c_str(), kWifiPassword.c_str());
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    blinkError(5);
    ESP.restart();
    Serial.println("restarting");
  }

  IPAddress ha_ip;
  ha_ip.fromString(kHomeAssistantIp.c_str());

  mqttClient.setId(kRoomName.c_str());
  mqttClient.setUsernamePassword(kMqttUser.c_str(), kMqttPassword.c_str());

  if (!mqttClient.connect(ha_ip, kMqttPort)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    blinkError(5);
    ESP.restart();
    Serial.println("restarting");
  }

  temperature_sensor.SetNameAndUniqueId("Balkon Temperature", "balkon");
  temperature_sensor.SetExpireTimeout(std::chrono::seconds(3));
  temperature_sensor.SetDeviceInfo("balkon");
  temperature_sensor.SetSensorData(SensorDeviceClass::kTemperature, "°C");
}

void loop() {
  Serial.println("I am working");

  mqttClient.poll();


  const auto config_topic = temperature_sensor.GetConfigTopic();
  const auto config_payload = temperature_sensor.GetConfigString();
  mqttClient.beginMessage(config_topic.c_str(), false);
  mqttClient.print(config_payload.c_str());
  mqttClient.endMessage();

  Serial.print("Sent config message to: ");
  Serial.println(config_topic.c_str());
  Serial.println(config_payload.c_str());
  Serial.println("-------------------");

  temperature_sensor.SetSensorValue(9.9);
  const auto state_topic = temperature_sensor.GetStateTopic();
  const auto state_payload = temperature_sensor.GetStateString();
  mqttClient.beginMessage(state_topic.c_str(), false);
  mqttClient.print(state_payload.c_str());
  mqttClient.endMessage();

  Serial.println("Sent state message to: ");
  Serial.println(state_topic.c_str());
  Serial.println(state_payload.c_str());
  Serial.println("-------------------");


  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
