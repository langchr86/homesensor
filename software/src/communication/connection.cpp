#include "connection.h"

Connection::Connection(IPAddress home_assistant_ip, uint16_t mqtt_port)
    : logger_("ConnectionHelper"), wifi_(&WiFi), mqtt_client_(home_assistant_ip, mqtt_port, wifi_client_)
{
}

bool Connection::Init(IPAddress own_static_ip, IPAddress gateway_ip, IPAddress subnet_mask, size_t mqtt_buffer_size)
{
    if (wifi_->mode(WIFI_STA) == false)
    {
        logger_.LogError("Failed to setup WIFI mode");
        return false;
    }
    if (wifi_->config(own_static_ip, gateway_ip, subnet_mask) == false)
    {
        logger_.LogError("Failed to setup static WIFI IP config");
        return false;
    }

    if (mqtt_client_.setBufferSize(mqtt_buffer_size) == false)
    {
        logger_.LogError("Failed to set MQTT buffer size");
        return false;
    }

    return true;
}

bool Connection::Connect(const char *host_name, const char *wifi_ssid, const char *wifi_password, const char *mqtt_user, const char *mqtt_password)
{
    wifi_->begin(wifi_ssid, wifi_password);
    if (wifi_->waitForConnectResult() != WL_CONNECTED)
    {
        logger_.LogError("WIFI connection failed");
        return false;
    }

    if (wifi_->setHostname(host_name) == false)
    {
        logger_.LogError("Failed to setup own hostname");
        return false;
    }

    if (mqtt_client_.connect(host_name, mqtt_user, mqtt_password) == false)
    {
        logger_.LogError("MQTT connection failed! Error code = %i", mqtt_client_.getWriteError());
        return false;
    }

    return true;
}

void Connection::Disconnect()
{
    mqtt_client_.disconnect();
    if (wifi_->disconnect() == false)
    {
        logger_.LogError("Failed to disconnect WIFI");
    }
}

bool Connection::Publish(const char *topic, const char *payload)
{
    return mqtt_client_.publish(topic, payload);
}
