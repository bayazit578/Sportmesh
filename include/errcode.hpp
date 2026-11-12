#pragma once

#include <cstdint>

#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

enum class ErrCode : std::uint32_t
{
  kOk,
  kFail,
  kPeripheralNotDetected,
  kAddrInvalid,
  kRouteTableEmpty,
  kStationTypeMismatch,
  kLoraMesher,
  kRTOS
};

__attribute__((always_inline)) 
inline std::uint32_t ErrCodeToInt(ErrCode code)
{
  return static_cast<std::uint32_t>(code);
}

END_SPORTMESH_NAMESPACE
