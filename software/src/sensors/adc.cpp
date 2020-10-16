#include "adc.h"

#include <cmath>

ADC::ADC(uint8_t pin, float voltage_divider, uint8_t bit_width, float max_voltage)
    : logger_("ADC"), pin_(pin), voltage_divider_(voltage_divider), max_value_(4095), max_voltage_(max_voltage)
{
    SetBitWidth(bit_width);
    analogSetPinAttenuation(pin, adc_attenuation_t::ADC_11db);
}

float ADC::ReadVoltage() const
{
    float adc_value = analogRead(pin_);
    logger_.LogDebug("Read ADC value: %f", adc_value);
    return adc_value / static_cast<float>(max_value_) * max_voltage_ * voltage_divider_;
}

bool ADC::SetBitWidth(uint8_t bit_width)
{
    if (bit_width >= 9 && bit_width <= 12)
    {
        analogSetWidth(bit_width);
        max_value_ = exp2f(bit_width) - 1;
        return true;
    }
    else
    {
        return false;
    }
}
