#pragma once

#include <cstdint>

#include "loramesher.hpp"

namespace config {

constexpr std::int8_t kPinCs   = 15;
constexpr std::int8_t kPinRst  = 16;
constexpr std::int8_t kPinIrq  = 26;
constexpr std::int8_t kPinBusy = 17;
constexpr std::int8_t kPinSck  = 18;
constexpr std::int8_t kPinMiso = 19;
constexpr std::int8_t kPinMosi = 23;

constexpr loramesher::RadioType kRadioType = loramesher::RadioType::kSx1262;

constexpr float    kFrequency       = 869.900F; // MHz — EU868 band
constexpr uint8_t  kSpreadingFactor = 7U;       // SF7–SF12: higher = more range, slower
constexpr float    kBandwidth       = 125.0F;   // kHz — 125/250/500
constexpr uint8_t  kCodingRate      = 7U;       // 5–8: higher = more error correction
constexpr int8_t   kPower           = 6;        // dBm — transmit power
constexpr uint8_t  kSyncWord        = 20U;      // Network identifier (0–255)
constexpr bool     kCrc             = true;     // Enable CRC checking
constexpr uint16_t kPreambleLength  = 8U;       // Preamble length in symbols

} // namespace config
