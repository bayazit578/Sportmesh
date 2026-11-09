#pragma once

#include <cstdint>

#include "loramesher.hpp"

namespace config {

// Lora SPI on HSPI (2)
constexpr uint8_t kPinRst  = 16;
constexpr uint8_t kPinDio0 = 22;
constexpr uint8_t kPinBusy = 17;
constexpr uint8_t kPinCs   = 5;
constexpr uint8_t kPinSck  = 18;
constexpr uint8_t kPinMiso = 19;
constexpr uint8_t kPinMosi = 23;

// Reader SPI on VSPI (3)
constexpr uint8_t kReaderPinRst  = 25;
constexpr uint8_t kReaderPinIrq  = 26;
constexpr uint8_t kReaderPinMiso = 12;
constexpr uint8_t kReaderPinMosi = 13;
constexpr uint8_t kReaderPinSck  = 14;
constexpr uint8_t kReaderPinCs   = 15;

constexpr uint8_t kBtn1Pin = 23;
constexpr uint8_t kBtn2Pin = 34;

// I2C
constexpr uint8_t kI2cClk = 32;
constexpr uint8_t kI2cDio = 33;

constexpr uint8_t kDisplayDigitCnt = 4;

constexpr uint8_t kVMeas = 35;

constexpr loramesher::RadioType kRadioType = loramesher::RadioType::kSx1262;

constexpr float    kFrequency       = 869.900F; // MHz — EU868 band
constexpr uint8_t  kSpreadingFactor = 7U;       // SF7–SF12: higher = more range, slower
constexpr float    kBandwidth       = 125.0F;   // kHz — 125/250/500
constexpr uint8_t  kCodingRate      = 7U;       // 5–8: higher = more error correction
constexpr int8_t   kPower           = 6;        // dBm — transmit power
constexpr uint8_t  kSyncWord        = 20U;      // Network identifier (0–255)
constexpr bool     kCrc             = true;     // Enable CRC checking
constexpr uint16_t kPreambleLength  = 8U;       // Preamble length in symbols

constexpr const std::size_t kUidSize_byte = 10;

} // namespace config
