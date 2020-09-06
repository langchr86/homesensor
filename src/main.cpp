#include <string>

#include <Arduino.h>
#include <WiFi.h>

static const std::string kRoomName = "balkon";
static const std::string kWifiSsid = "";
static const std::string kWifiPassword = "";

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
  }
}


/*
void setup() {
  App.set_name(kRoomName);

  App.init_log();

  auto* wifi = App.init_wifi(kWifiSsid, kWifiPassword);
  // wifi->set_use_address("192.168.0.11");
  // wifi->set_power_save_mode(WiFiPowerSaveMode::WIFI_POWER_SAVE_HIGH);

  App.init_mqtt("192.168.0.87", "mqtt_user", "mgs237");

  App.make_sht3xd_sensor(kRoomName + " Temperature", kRoomName + " Humidity", 10);
  App.make_status_binary_sensor(kRoomName + " Node Status");
  App.make_restart_switch(kRoomName + " Restart");

  App.setup();
}
*/

void loop() {
  Serial.println("I am working");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000);

  // App.loop();
}