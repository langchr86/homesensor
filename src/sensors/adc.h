#pragma once

#include <cstdint>

#include <Arduino.h>

#include "utils/logger.h"

class ADC
{
public:
    ADC(uint8_t pin = A0, float voltage_divider = 2.0, uint8_t bit_width = 12, float max_voltage = 3.6f);

    float ReadVoltage() const;

    bool SetBitWidth(uint8_t bit_width);

private:
    Logger logger_;
    uint8_t pin_;
    float voltage_divider_;
    uint32_t max_value_;
    float max_voltage_;
};
