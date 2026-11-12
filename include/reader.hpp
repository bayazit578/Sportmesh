#pragma once

#include <cstdint>

#include <MFRC522.h>

#include "errcode.hpp"
#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

struct ReaderConfig
{
  std::uint8_t pin_irq;
  std::uint8_t pin_rst;
  std::uint8_t pin_sck;
  std::uint8_t pin_miso;
  std::uint8_t pin_mosi;
  std::uint8_t pin_cs;
};

class Reader
{
public:

  // @param read_register_task task that will be notified when read appears
  // @param read_queue queue where PunchEvent will be put
  // @param cfg pin configuration
  Reader(TaskHandle_t read_register_task, 
         QueueHandle_t read_queue,
         ReaderConfig cfg);

  ErrCode Start();

  Reader(Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  Reader(Reader&&) noexcept = delete;
  Reader& operator=(Reader&&) noexcept = delete;

private:
  ReaderConfig m_pin_cfg;

  MFRC522 m_mfrc522;

  TaskHandle_t  m_read_register_task;
  QueueHandle_t m_read_queue;

  friend void taskOnReadInterrupt(void *param);
  friend void taskTriggerReader(void* param);
};

END_SPORTMESH_NAMESPACE
