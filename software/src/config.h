#pragma once

#include <cstddef>

#include <chrono>

#include <WiFi.h>

#include "infrastructure/device_config.h"

static constexpr size_t kWireSpeedHz = 100000;

static constexpr std::chrono::seconds kDefaultReadoutInterval = std::chrono::minutes(1);
static constexpr std::chrono::seconds kMaxReadoutInterval = std::chrono::minutes(30);
static_assert(kMaxReadoutInterval > kDefaultReadoutInterval, "kMaxReadoutInterval needs to be bigger then kDefaultReadoutInterval");

static constexpr char kWifiSsid[] = "";
static constexpr char kWifiPassword[] = "";

static const IPAddress kHomeAssistantIp(192, 168, 0, 8);
static const IPAddress kGatewayIp(192, 168, 0, 1);
static const IPAddress kSubnetMask(255, 255, 255, 0);

static constexpr char kMqttUser[] = "";
static constexpr char kMqttPassword[] = "";
static constexpr uint16_t kMqttPort = 1883;
static constexpr size_t kMqttMaxMessageSize = 512;

static const DeviceConfig config[] = {
    DeviceConfig(DeviceType::kShtc3, IPAddress(192, 168, 0, 11), "Balkon", "balkon"),
    DeviceConfig(DeviceType::kShtc3, IPAddress(192, 168, 0, 12), "Front", "front"),
    DeviceConfig(DeviceType::kScd30, IPAddress(192, 168, 0, 13), "Wohnen", "wohnen"),
    DeviceConfig(DeviceType::kScd30, IPAddress(192, 168, 0, 14), "Büro", "buro"),
    DeviceConfig(DeviceType::kScd30, IPAddress(192, 168, 0, 15), "Schlafen", "schlafen"),
    DeviceConfig(DeviceType::kScd30, IPAddress(192, 168, 0, 16), "Freizeit", "freizeit"),
    DeviceConfig(DeviceType::kScd30, IPAddress(192, 168, 0, 17), "Sport", "sport"),
};

static constexpr size_t kDeviceConfigIndex = 0;
static_assert(kDeviceConfigIndex < sizeof(config), "kDeviceConfigIndex needs to be a valid index of the config array");
