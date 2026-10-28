#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include "config.hpp"

struct PunchEvent {
  uint8_t uid[config::kUidSize_byte];
  uint16_t time;

  void serialize(std::vector<uint8_t>& dat)
  {
    // FIXME
    std::memcpy(dat.data(), uid, sizeof(uid));
  }
};

