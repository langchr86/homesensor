#include <string>

#include <Arduino.h>
#include <WiFi.h>

#include <ArduinoMqttClient.h>

static const std::string kRoomName = "balkon";
static const std::string kWifiSsid = "";
static const std::string kWifiPassword = "";

static const std::string kHomeAssistantIp = "192.168.0.87";
static const uint16_t kMqttPort = 1883;
static const std::string kMqttUser = "mqtt_user";
static const std::string kMqttPassword = "mgs237";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

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
}

void loop() {
  Serial.println("I am working");

  mqttClient.poll();

  mqttClient.beginMessage("home/balkon/temperature", true);
  mqttClient.print("39.2");
  mqttClient.endMessage();

  Serial.println("Sent message to MQTT");

  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
