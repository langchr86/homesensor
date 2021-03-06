#include "power.h"

#include <Esp.h>
#include <esp_sleep.h>

Power::Power() : logger_("Power") {}

void Power::SetOption(esp_sleep_pd_domain_t domain, esp_sleep_pd_option_t option)
{
    const auto result = esp_sleep_pd_config(domain, option);
    if (result != ESP_OK)
    {
        logger_.LogError("failed to set power option: domain=%i option=%i", domain, option);
    }
}

void Power::DeepSleepNow(const std::chrono::seconds &duration)
{
    SetOption(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    logger_.LogInfo("going to deep sleep for %u seconds", static_cast<int>(duration.count()));
    esp_deep_sleep(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}

bool Power::LightSleepNow(const std::chrono::microseconds &duration)
{
    const std::chrono::microseconds kSettingSleep(1000);
    if (duration <= kSettingSleep)
    {
        logger_.LogError("duration is too small");
        return false;
    }

    const std::chrono::microseconds real_value = duration - kSettingSleep;
    if (esp_sleep_enable_timer_wakeup(real_value.count()) != ESP_OK)
    {
        logger_.LogError("failed to set wakeup option");
        return false;
    }
    delayMicroseconds(kSettingSleep.count());

    if (esp_light_sleep_start() != ESP_OK)
    {
        logger_.LogError("failed to start light sleep");
        return false;
    }
    return true;
}

void Power::Reboot()
{
    const auto kWaitBeforeReboot = std::chrono::seconds(3);
    logger_.LogWarning("Restarting in %u seconds", static_cast<int>(kWaitBeforeReboot.count()));
    LightSleepNow(kWaitBeforeReboot);
    ESP.restart();
}
