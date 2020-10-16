#pragma once

#include <chrono>

class Led
{
public:
    static void Enable();
    static void Disable();
    static void FlashFor(std::chrono::milliseconds duration = std::chrono::milliseconds(2000), std::chrono::milliseconds interval = std::chrono::milliseconds(250));

private:
    static void Init();
};
