#pragma once

#include <cmath>

class BatteryCalculation
{
public:
    static float CapacityLevelPercent(float voltage, float low_voltage = 3.5, float high_voltage = 4.1);
};