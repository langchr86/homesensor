#include <chrono>
#include <string>

#include <ArduinoJson.h>

#include "sensor_device_class.h"

class AutoDiscovery
{
  static const std::string kTopicBase;

public:
  AutoDiscovery();

  void SetUniqueName(const std::string &name);
  void SetNameAndUniqueId(const std::string &name, const std::string & unique_id);
  void SetAvailablitiy();
  void SetDeviceInfo(const std::string& unique_device_name);
  void SetSensorData(SensorDeviceClass device_class, const std::string& unit_of_measurement);
  void SetExpireTimeout(const std::chrono::seconds& timeout);

  std::string GetConfigTopic() const;
  std::string GetStateTopic() const;
  std::string GetConfigString() const;

private:
  std::string config_topic_;
  std::string state_topic_;
  StaticJsonDocument<JSON_OBJECT_SIZE(32)> root_;
};
