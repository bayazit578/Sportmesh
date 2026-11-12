#pragma once

#include <cstdint>

enum class Error : std::uint32_t
{
  kOk,
  kFail,
  kPeripheralNotDetected
};
