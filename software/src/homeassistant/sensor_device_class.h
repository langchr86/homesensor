#pragma once

#include <Arduino.h>

enum class SensorDeviceClass
{
    kNone,
    kBattery,
    kHumidity,
    kIlluminance,
    kSignalStrength,
    kTemperature,
    kPower,
    kPressure,
    kTimestamp,
};

String ToString(SensorDeviceClass device_class);
