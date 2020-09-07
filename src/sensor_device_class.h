#include <string>

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

namespace std {

std::string to_string(SensorDeviceClass device_class);

}
