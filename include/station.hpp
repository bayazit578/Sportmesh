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

class Station
{
public:
  Station(lm::PinConfig pin_config, 
          lm::RadioConfig radio_config,
          StationType station_type = kCheckpoint);

  int start();

  int register_punch();

private:

  void m_data_callback(lm::AddressType source, 
                       const std::vector<uint8_t>& data);

  void m_discover_base_station();

  std::unique_ptr<lm::LoraMesher> m_mesher;
  lm::LoRaMeshProtocolConfig m_protocol_config;

  lm::AddressType base_station_addr;

  static constexpr const char* kTag = "station";

};
