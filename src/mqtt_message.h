#pragma once

class MqttMessage
{
public:
    MqttMessage() {}
    MqttMessage(String topic, String payload) : topic_(topic), payload_(payload) {}

    String GetTopic() const { return topic_; }
    String GetPayload() const { return payload_; }

private:
    String topic_;
    String payload_;
};
