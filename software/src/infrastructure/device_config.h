#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "infrastructure/device_type.h"

struct DeviceConfig
{
    DeviceConfig(DeviceType type, IPAddress ip_address, String name, String unique_id);

    DeviceType type;
    IPAddress ip_address;
    String name;
    String unique_id;
};
