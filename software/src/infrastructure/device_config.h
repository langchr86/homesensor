#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "infrastructure/device_type.h"

struct DeviceConfig
{
    DeviceType type;
    String name;
    String unique_id;
    IPAddress ip_address;
};
