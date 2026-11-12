#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

#include "config.hpp"
#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

struct PunchEvent {
  uint8_t uid[config::kUidSize_byte];
  uint16_t time;

  void serialize(std::vector<uint8_t>& dat)
  {
    // FIXME
    std::memcpy(dat.data(), uid, sizeof(uid));
  }
};

END_SPORTMESH_NAMESPACE
