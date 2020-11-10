#pragma once

#include <WiFi.h>

#include <PubSubClient.h>

#include "utils/logger.h"

class Connection
{
public:
    Connection(IPAddress home_assistant_ip, uint16_t mqtt_port, IPAddress gateway_ip, IPAddress subnet_mask, size_t mqtt_buffer_size);

    bool Init(IPAddress own_static_ip);
    bool Connect(const char *host_name, const char *wifi_ssid, const char *wifi_password, const char *mqtt_user, const char *mqtt_password);
    void Disconnect();

    bool Publish(const char *topic, const char *payload);

private:
    Logger logger_;
    WiFiClass *wifi_;
    WiFiClient wifi_client_;
    PubSubClient mqtt_client_;

    IPAddress gateway_ip_;
    IPAddress subnet_mask_;
    size_t mqtt_buffer_size_;
};
