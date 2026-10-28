#pragma once
#include "Arduino.h"
#include "MFRC522.h"

class Reader
{
public:

  // @param read_register_task task that will be notified when read appears
  // @param read_queue queue where PunchEvent will be put
  Reader(TaskHandle_t read_register_task, 
         QueueHandle_t read_queue);

  // Non-copyable
  Reader(Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  // Movable
  Reader(Reader&&) noexcept = default;
  Reader& operator=(Reader&&) noexcept = default;

private:
  MFRC522 m_mfrc522;
  TaskHandle_t m_read_register_task;
  QueueHandle_t m_read_queue;

  friend void taskOnReadInterrupt(void *param);
  friend void taskTriggerReader(void* param);
};
