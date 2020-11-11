#include "infrastructure/device_config.h"

DeviceConfig::DeviceConfig(DeviceType type, IPAddress ip_address, String name, String unique_id)
    : type(type), ip_address(ip_address), name(name), unique_id(unique_id) {}
