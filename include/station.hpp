#pragma once

#include <Arduino.h>
#include <memory>

#include "loramesher.hpp"

#include "errcode.hpp"
#include "punch_event.hpp"
#include "sm_namespace.hpp"

namespace lm = loramesher;

BEGIN_SPORTMESH_NAMESPACE

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

  Station(Station&) = delete;
  Station& operator=(Station&) = delete;

  Station(Station&&) noexcept = delete;
  Station& operator=(Station&&) noexcept = delete;

  ErrCode Start();

  ErrCode RegisterPunch(PunchEvent msg);

  ErrCode DiscoverBaseStation();

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

END_SPORTMESH_NAMESPACE
