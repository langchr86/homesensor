#include "led.h"

#include <Arduino.h>

void Led::Enable()
{
    Init();
    digitalWrite(LED_BUILTIN, HIGH);
}

void Led::Disable()
{
    Init();
    digitalWrite(LED_BUILTIN, LOW);
}

void Led::FlashFor(std::chrono::milliseconds duration, std::chrono::milliseconds interval)
{
    Init();

    assert(duration >= 2 * interval);
    const size_t iteration_count = duration / (2 * interval);

    for (size_t i = 0; i < iteration_count; ++i)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(interval.count());
        digitalWrite(LED_BUILTIN, LOW);
        delay(interval.count());
    }
}

void Led::Init()
{
    static bool initialized = false;
    if (initialized)
    {
        return;
    }
    pinMode(LED_BUILTIN, OUTPUT);
    initialized = true;
}
