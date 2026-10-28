#pragma once

#include <memory>
#include "loramesher.hpp"

namespace lm = loramesher;

enum StationType : std::uint8_t 
{
  kBaseStation  = 0x02,
  kCheckpoint   = 0x04,
  kRetranslator = 0x08,
};

struct PunchMessage {
  uint16_t id;
  uint16_t time;

  void serialize(std::vector<uint8_t>& dat)
  {
    uint16_t payload[] = { id, time };
    std::size_t payload_size = sizeof(payload) / sizeof(payload[0]);
    dat.resize(payload_size);
    std::memcpy(dat.data(), payload, payload_size);
  }
};

class Station
{
public:
  Station(lm::PinConfig pin_config, 
          lm::RadioConfig radio_config,
          StationType station_type = kCheckpoint);

  int start();

  int register_punch(PunchMessage msg);

private:

  void m_data_callback(lm::AddressType source, 
                       const std::vector<uint8_t>& data);

  void m_discover_base_station();

  std::unique_ptr<lm::LoraMesher> m_mesher;

  lm::AddressType base_station_addr;

};
