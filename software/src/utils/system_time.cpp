#include "system_time.h"

#include <ctime>
#include <cstdio>

#include <chrono>

char SystemTime::time_string_[SystemTime::kMaxTimeStringSize];

const char *SystemTime::GetUptimeString()
{
  using std::chrono::microseconds;

  const auto time_since_epoch = std::chrono::system_clock::now().time_since_epoch();
  const auto us_since_epoch = std::chrono::duration_cast<microseconds>(time_since_epoch).count();
  const auto secs = static_cast<std::time_t>(us_since_epoch / microseconds::period::den);
  const auto us = static_cast<std::size_t>(us_since_epoch % microseconds::period::den);

  const tm *ptm = std::localtime(&secs);

  sprintf(time_string_, "%03id %02i:%02i:%02i.%06u", ptm->tm_yday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, us);

  return time_string_;
}
