#pragma once

#include <chrono>

#include "utils/logger.h"

class Power
{
public:
    Power();

    void SetOption(esp_sleep_pd_domain_t domain, esp_sleep_pd_option_t option);
    void DeepSleepNow(const std::chrono::seconds &duration);
    void Reboot();

private:
    Logger logger_;
};
