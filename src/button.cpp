#include "button.hpp"

#include "Arduino.h"
#include "driver/gpio.h"

BEGIN_SPORTMESH_NAMESPACE

Button::Button(const ButtonConfig& cfg)
  : m_cfg{cfg} {}

ErrCode Button::Start(QueueHandle_t events) {
  if (m_process_task_handle) {
    return events == m_push_events ? ErrCode::kOk : ErrCode::kFail;
  }
  if (!events || !GPIO_IS_VALID_GPIO(m_cfg.pin) ||
    (m_cfg.internal_pullup && m_cfg.pin >= 34) ||
    pdMS_TO_TICKS(m_cfg.long_press_ms) == 0 ||
    pdMS_TO_TICKS(m_cfg.long_press_ms) == portMAX_DELAY) {
    return ErrCode::kFail;
  }

  m_push_events = events;
  pinMode(m_cfg.pin, m_cfg.internal_pullup ? INPUT_PULLUP : INPUT);
  if (xTaskCreate(taskProcessInput, "button", 4096, this, 1,
    &m_process_task_handle) != pdPASS) {
    m_push_events = nullptr;
    m_process_task_handle = nullptr;
    return ErrCode::kRTOS;
  }
  attachInterruptArg(m_cfg.pin, onInputChanged, this, CHANGE);
  return ErrCode::kOk;
}

void ARDUINO_ISR_ATTR Button::onInputChanged(void* param) {
  auto* button = static_cast<Button*>(param);
  BaseType_t task_woken = pdFALSE;
  vTaskNotifyGiveFromISR(button->m_process_task_handle, &task_woken);
  if (task_woken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

void Button::taskProcessInput(void* param) {
  auto* button = static_cast<Button*>(param);
  const TickType_t long_press = pdMS_TO_TICKS(button->m_cfg.long_press_ms);
  bool pressed = false;
  bool long_sent = false;
  TickType_t pressed_at = 0;

  for (;;) {
    TickType_t timeout = portMAX_DELAY;
    if (pressed && !long_sent) {
      const TickType_t elapsed = xTaskGetTickCount() - pressed_at;
      timeout = elapsed >= long_press ? 0 : long_press - elapsed;
    }

    ulTaskNotifyTake(pdTRUE, timeout);
    const bool next_pressed = digitalRead(button->m_cfg.pin)
      == (button->m_cfg.active_low ? LOW : HIGH);
    if (next_pressed != pressed) {
      pressed = next_pressed;
      if (pressed) {
        pressed_at = xTaskGetTickCount();
        long_sent = false;
      } else if (!long_sent) {
        const TickType_t elapsed = xTaskGetTickCount() - pressed_at;
        const ButtonEvent event {
          button->m_cfg.btnId,
          elapsed >= long_press 
                     ? ButtonEvent::kLong 
                     : ButtonEvent::kShort
        };
        xQueueSend(
          button->m_push_events, 
          &event, 
          (TickType_t) 0);
      }
    } else if (pressed && !long_sent &&
      xTaskGetTickCount() - pressed_at >= long_press) {
      long_sent = true;
      const ButtonEvent event { 
        button->m_cfg.btnId, 
        ButtonEvent::kLong 
      };
      xQueueSend(
        button->m_push_events, 
        &event, 
        (TickType_t) 0);
    }
  }
}

END_SPORTMESH_NAMESPACE
