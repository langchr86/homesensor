#pragma once

#include <cstddef>

class SystemTime
{
    static constexpr size_t kMaxTimeStringSize = 32;

public:
    static const char *GetUptimeString();

private:
    static char time_string_[kMaxTimeStringSize];
};
