#include <string>

#include <Arduino.h>

#include <SoftWire.h>

#include "communication/connection.h"
#include "infrastructure/device_factory.h"
#include "sensors/adc.h"
#include "utils/led.h"
#include "utils/logger.h"
#include "utils/power.h"

#include "config.h"

std::shared_ptr<SensorInterface> sensor = nullptr;

RTC_DATA_ATTR bool sensor_hardware_initialized = false;
RTC_DATA_ATTR size_t boot_count = 0;
RTC_DATA_ATTR size_t failed_boot_count = 0;
RTC_DATA_ATTR size_t failed_consecutive_boots = 0;
RTC_DATA_ATTR std::chrono::seconds readout_interval = kDefaultReadoutInterval;
RTC_DATA_ATTR std::chrono::seconds max_readout_interval = readout_interval;

RTC_DATA_ATTR bool calibration_active = false;
RTC_DATA_ATTR std::chrono::seconds remaining_calibration_time;

bool InitWire(Logger *logger, TwoWire *wire)
{
  wire->begin();
  wire->setClock(kWireSpeedHz);

  return true;
}

void Sleep(Power *power, Logger *logger)
{
  if (sensor->LowPower())
  {
    logger->LogWarning("Force kMaxReadOutInterval because of low power");
    power->DeepSleepNow(kMaxReadoutInterval);
  }
  else
  {
    power->DeepSleepNow(readout_interval);
  }
}

void ErrorHappened(Connection *connection, Power *power, Logger *logger)
{
  if (connection != nullptr)
  {
    connection->Disconnect();
  }

  failed_boot_count++;
  failed_consecutive_boots++;

  if (failed_consecutive_boots > 3)
  {
    readout_interval *= 2;
    failed_consecutive_boots = 0;

    if (readout_interval > kMaxReadoutInterval)
    {
      readout_interval = kMaxReadoutInterval;
    }

    if (max_readout_interval < readout_interval)
    {
      max_readout_interval = readout_interval;
    }

    logger->LogInfo("Increased read out interval to %u seconds", static_cast<int>(readout_interval.count()));
  }

  Sleep(power, logger);
}

void EndWithNoError(Connection *connection, Power *power, Logger *logger)
{
  failed_consecutive_boots = 0;
  readout_interval = kDefaultReadoutInterval;
  connection->Disconnect();
  Sleep(power, logger);
}

void setup()
{
  boot_count++;
  std::string mode = "norm";

  Led::Disable();
  Power power;

  Logger logger("Main");
  logger.SetMaxLevel(LOG_DEBUG);

  static constexpr uint8_t kCalibrationPin = GPIO_NUM_4;
  static constexpr std::chrono::seconds kMinCalibrationTime = std::chrono::minutes(10);
  pinMode(kCalibrationPin, INPUT_PULLUP);
  if (boot_count == 1 && digitalRead(kCalibrationPin) == LOW)
  {
    calibration_active = true;
    remaining_calibration_time = 10 * kDefaultReadoutInterval;
    if (remaining_calibration_time < kMinCalibrationTime)
    {
      remaining_calibration_time = kMinCalibrationTime;
    }
    logger.LogDebug("Initialized sensor calibration");
    Led::FlashFor(std::chrono::seconds(4), std::chrono::seconds(1));
  }

  auto *wire = &Wire;
  if (InitWire(&logger, wire) == false)
  {
    logger.LogError("Failed to initialize Wire connection");
    ErrorHappened(nullptr, &power, &logger);
  }
  logger.LogDebug("Success: InitWire");

  ADC adc(A0);
  adc.SetBitWidth(9);

  Connection connection(kHomeAssistantIp, kMqttPort, kGatewayIp, kSubnetMask, kMqttMaxMessageSize);

  const std::chrono::seconds expire_timeout(3 * kDefaultReadoutInterval);
  DeviceFactory factory(readout_interval, expire_timeout);

  sensor = factory.CreateDevice(config[kDeviceConfigIndex], &adc, wire, &connection, &power);

  if (sensor == nullptr)
  {
    logger.LogError("Failed to create sensor device");
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: factory.CreateDevice");

  if (sensor_hardware_initialized == false)
  {
    mode += "+hwinit";
    if (sensor->HardwareInitialization(kDefaultReadoutInterval) == false)
    {
      logger.LogError("Sensor hardware initialization failed");
      ErrorHappened(&connection, &power, &logger);
    }
    sensor_hardware_initialized = true;
    Led::FlashFor();
    logger.LogDebug("Success: sensor.HardwareInitialization");
  }

  if (calibration_active && remaining_calibration_time <= std::chrono::seconds::zero())
  {
    calibration_active = false;
    if (sensor->ForceCalibrationNow() == false)
    {
      logger.LogError("Sensor calibration failed");
      ErrorHappened(&connection, &power, &logger);
    }
    logger.LogDebug("Success: sensor.ForceCalibrationNow");
  }

  if (calibration_active)
  {
    mode += "+calib";
    remaining_calibration_time -= kDefaultReadoutInterval;
  }

  if (sensor->SensorReadLoop() == false)
  {
    logger.LogError("Sensor read out loop failed");
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: sensor.SensorReadLoop");

  sensor->SetDebugInfos(mode.c_str(), boot_count, failed_boot_count, max_readout_interval);

  if (connection.Init() == false)
  {
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: connection.Init");

  if (connection.Connect(config[kDeviceConfigIndex].unique_id.c_str(), kWifiSsid, kWifiPassword, kMqttUser, kMqttPassword) == false)
  {
    ErrorHappened(&connection, &power, &logger);
  }
  logger.LogDebug("Success: connection.Connect");

  if (boot_count == 1 || failed_consecutive_boots > 0) // only send HA config at first boot or after failed boot
  {
    if (sensor->SendHomeassistantConfig() == false)
    {
      ErrorHappened(&connection, &power, &logger);
    }
  }

  if (sensor->SendHomeassistantState() == false)
  {
    ErrorHappened(&connection, &power, &logger);
  }

  EndWithNoError(&connection, &power, &logger);
}

// we use only setup() function
void loop() {}
