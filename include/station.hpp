#pragma once

#include <memory>
#include "Arduino.h"
#include "loramesher.hpp"
#include "punch_event.hpp"

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

  // Non-copyable
  Station(Station&) = delete;
  Station& operator=(Station&) = delete;

  // Movale
  Station(Station&&) noexcept = default;
  Station& operator=(Station&&) noexcept = default;

  int Start();

  int RegisterPunch(PunchEvent msg);

  int DiscoverBaseStation();

  TaskHandle_t getReadHandler() { return m_register_task_handle; }
  QueueHandle_t getReadQueue() { return m_read_queue; }

private:

  void DataCallback(lm::AddressType source, 
                       const std::vector<uint8_t>& data);

  std::unique_ptr<lm::LoraMesher> m_mesher;

  lm::AddressType m_base_station_addr;

  TaskHandle_t m_register_task_handle;

  QueueHandle_t m_read_queue;

  friend void taskDiscoverBaseStation(void* param);
  friend void taskRegisterPunch(void* param);

};
