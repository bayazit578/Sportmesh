#include "reader.hpp"

#include "Arduino.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include <array>

#include "config.hpp"
#include "punch_event.hpp"

static constexpr const char* kTag = "reader";

static constexpr std::uint32_t kMFRCTriggerInterval_ms = 300;


static void IRAM_ATTR intRead(void* param);

void taskTriggerReader(void* param);

void taskOnReadInterrupt(void *param);


Reader::Reader(TaskHandle_t read_register,
               QueueHandle_t read_queue)
  : m_mfrc522{ config::kReaderPinCs, config::kReaderPinRst },
    m_read_register_task{ read_register },
    m_read_queue{ read_queue }
{
  pinMode(config::kReaderPinIrq, INPUT_PULLUP);
  SPI.begin(config::kReaderPinSck, config::kReaderPinMiso,
            config::kReaderPinMosi, config::kReaderPinCs);
  m_mfrc522.PCD_Init();

  const byte version = m_mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  if (version != 0x91 && version != 0x92)
    ESP_LOGE(kTag, "RC522 not detected (version 0x%02X)", version);

  ESP_LOGI(kTag, "RC522 version 0x%02X", version);

  // Allow interrupt to be propagated on IRQ pin
  m_mfrc522.PCD_WriteRegister(m_mfrc522.ComIEnReg, 0xA0);

  BaseType_t res = pdFAIL;

  // Trigger reader periodically
  res = xTaskCreate(taskTriggerReader, "trigger reader",
                    4096, this,
                    1, nullptr);

  if (res != pdPASS)
    ESP_LOGE(kTag, "Failed to create reader trigger task");
  
  xTaskHandle on_read_int_task_handle = nullptr;
  res = xTaskCreate(taskOnReadInterrupt, "on read interrupt",
                    4096, this,
                    2, &on_read_int_task_handle);

  attachInterruptArg(digitalPinToInterrupt(config::kReaderPinIrq), 
                     intRead, 
                     on_read_int_task_handle, 
                     FALLING);
}

static void IRAM_ATTR intRead(void* param)
{
  TaskHandle_t on_read_handle = static_cast<TaskHandle_t>(param);
  BaseType_t higher_priority_task_woken;
  vTaskNotifyGiveFromISR(on_read_handle, &higher_priority_task_woken);
  portYIELD_FROM_ISR(higher_priority_task_woken);
}

void taskOnReadInterrupt(void* param)
{
  Reader* reader = static_cast<Reader*>(param);
  PunchEvent event{};
  BaseType_t res{ pdFAIL };

  for (;;) {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
      // Disable interrupt for time of processing
      reader->m_mfrc522.PCD_WriteRegister(MFRC522::ComIEnReg, 0x80);

      bool selected = reader->m_mfrc522.PICC_ReadCardSerial();
      if (selected) {
        std::memcpy(&event.uid, &reader->m_mfrc522.uid.uidByte, config::kUidSize_byte);

        res = xQueueSend(reader->m_read_queue, 
                   static_cast<void*>(&event),
                   static_cast<TickType_t>(0)); 
        if (res != pdPASS)
          ESP_LOGE(kTag, "Failed to send msg to read queue");
        xTaskNotifyGive(reader->m_read_register_task);

        reader->m_mfrc522.PICC_HaltA();
      }

      // Clearing interrupt
      reader->m_mfrc522.PCD_WriteRegister(reader->m_mfrc522.ComIrqReg, 0x7F); 

      // Rearming interrupts
      reader->m_mfrc522.PCD_WriteRegister(reader->m_mfrc522.ComIEnReg, 0xA0); 
    }
  }
}

void taskTriggerReader(void* param)
{
  Reader* reader = static_cast<Reader*>(param);

  const TickType_t interval = pdMS_TO_TICKS(kMFRCTriggerInterval_ms);
  TickType_t last_wake = xTaskGetTickCount();

  for (;;) {
    ESP_LOGI(kTag, "Triggering reader");

    reader->m_mfrc522.PCD_WriteRegister(reader->m_mfrc522.FIFODataReg, reader->m_mfrc522.PICC_CMD_REQA);
    reader->m_mfrc522.PCD_WriteRegister(reader->m_mfrc522.CommandReg, reader->m_mfrc522.PCD_Transceive);
    reader->m_mfrc522.PCD_WriteRegister(reader->m_mfrc522.BitFramingReg, 0x87);

    vTaskDelayUntil(&last_wake, interval);
  }
}
