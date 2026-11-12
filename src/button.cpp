#include "button.hpp"

#include <utility>

#include "Arduino.h"
#include "driver/gpio.h"

BEGIN_SPORTMESH_NAMESPACE

Button::Button(ButtonConfig cfg, Callback on_brief, Callback on_long)
  : m_cfg{cfg}, m_on_brief{std::move(on_brief)},
    m_on_long{std::move(on_long)} {}

ErrCode Button::Start() {
  if (m_task) {
    return ErrCode::kOk;
  }
  if (!GPIO_IS_VALID_GPIO(m_cfg.pin) ||
    (m_cfg.internal_pullup && m_cfg.pin >= 34) ||
    pdMS_TO_TICKS(m_cfg.long_press_ms) == 0 ||
    pdMS_TO_TICKS(m_cfg.long_press_ms) == portMAX_DELAY) {
    return ErrCode::kFail;
  }

  m_events = xQueueCreate(8, sizeof(bool));
  if (!m_events) {
    return ErrCode::kRTOS;
  }
  pinMode(m_cfg.pin, m_cfg.internal_pullup ? INPUT_PULLUP : INPUT);
  const bool pressed = digitalRead(m_cfg.pin) == (m_cfg.active_low ? LOW : HIGH);
  xQueueSend(m_events, &pressed, 0);
  attachInterruptArg(m_cfg.pin, onInputChanged, this, CHANGE);
  if (xTaskCreate(taskProcessInput, "button", 512, this, 1, &m_task)
    != pdPASS) {
    detachInterrupt(m_cfg.pin);
    vQueueDelete(m_events);
    m_events = nullptr;
    m_task = nullptr;
    return ErrCode::kRTOS;
  }
  return ErrCode::kOk;
}

void ARDUINO_ISR_ATTR Button::onInputChanged(void* param) {
  auto* button = static_cast<Button*>(param);
  const bool pressed = digitalRead(button->m_cfg.pin)
    == (button->m_cfg.active_low ? LOW : HIGH);
  BaseType_t task_woken = pdFALSE;
  xQueueSendFromISR(button->m_events, &pressed, &task_woken);
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

    bool next_pressed;
    const bool received = xQueueReceive(button->m_events, &next_pressed, timeout)
      == pdPASS;
    if (!received) {
      long_sent = true;
      if (button->m_on_long) {
        button->m_on_long();
      }
    } else if (next_pressed != pressed) {
      pressed = next_pressed;
      if (pressed) {
        pressed_at = xTaskGetTickCount();
        long_sent = false;
      } else if (!long_sent) {
        const TickType_t elapsed = xTaskGetTickCount() - pressed_at;
        const auto& callback = elapsed >= long_press
          ? button->m_on_long : button->m_on_brief;
        if (callback) {
          callback();
        }
      }
    }
  }
}

END_SPORTMESH_NAMESPACE
