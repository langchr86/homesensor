#include "sensor_device_class.h"

#include <cassert>

String ToString(SensorDeviceClass device_class) {
    using SDC = SensorDeviceClass;
    switch(device_class) {
        case SDC::kNone: return "none";
        case SDC::kBattery: return "battery";
        case SDC::kHumidity: return "humidity";
        case SDC::kIlluminance: return "illuminance";
        case SDC::kSignalStrength: return "signal_strength";
        case SDC::kTemperature: return "temperature";
        case SDC::kPower: return "power";
        case SDC::kPressure: return "pressure";
        case SDC::kTimestamp: return "timestamp";
    }

    assert(false);
    return "";
}
