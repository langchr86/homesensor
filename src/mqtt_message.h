#pragma once

#include <Arduino.h>

class MqttMessage
{
public:
    MqttMessage() {}
    MqttMessage(String topic, String payload) : topic_(topic), payload_(payload) {}

    const char* GetTopic() const { return topic_.c_str(); }
    const char* GetPayload() const { return payload_.c_str(); }

private:
    String topic_;
    String payload_;
};
