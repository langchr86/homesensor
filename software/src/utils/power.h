#pragma once

#include <chrono>

#include "utils/logger.h"

class Power
{
public:
    Power();

    void SetOption(esp_sleep_pd_domain_t domain, esp_sleep_pd_option_t option);
    void DeepSleepNow(const std::chrono::seconds &duration);
    bool LightSleepNow(const std::chrono::microseconds &duration);
    void Reboot();

    template <class Predicate>
    bool WaitUntil(const std::chrono::microseconds &timeout, const std::chrono::microseconds &interval, const Predicate &predicate)
    {
        std::chrono::microseconds remaining = timeout;

        while (remaining > std::chrono::microseconds(0))
        {
            if (predicate())
            {
                return true;
            }
            LightSleepNow(interval);
            remaining -= interval;
        }

        return predicate();
    }

private:
    Logger logger_;
};
