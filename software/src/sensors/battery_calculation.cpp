#include "battery_calculation.h"

float BatteryCalculation::CapacityLevelPercent(float voltage, float low_voltage, float high_voltage)
{
    const float difference = high_voltage - low_voltage;
    const float percent_factor = 100.0;

    float level = ((voltage - low_voltage) / difference) * percent_factor;
    if (round(level) >= 100)
    {
        level = 100.0;
    }

    return level;
}
