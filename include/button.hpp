#pragma once

#include <cstdint>
#include <functional>

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "errcode.hpp"
#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

struct ButtonConfig {
  std::uint8_t pin;
  bool active_low = true;
  // The PCB has external pull-ups. GPIO34 does not support internal pull-ups.
  bool internal_pullup = false;
  std::uint32_t long_press_ms = 800;
};

class Button {
public:
  using Callback = std::function<void()>;

  Button(ButtonConfig cfg, Callback on_brief, Callback on_long);

  Button(const Button&) = delete;
  Button& operator=(const Button&) = delete;
  Button(Button&&) = delete;
  Button& operator=(Button&&) = delete;

  // Once started, this object must remain alive for the firmware's lifetime.
  // Callbacks run in the button task; keep them short.
  ErrCode Start();

private:
  static void ARDUINO_ISR_ATTR onInputChanged(void* param);
  static void taskProcessInput(void* param);

  ButtonConfig m_cfg;
  Callback m_on_brief;
  Callback m_on_long;
  TaskHandle_t m_task = nullptr;
  QueueHandle_t m_events = nullptr;
};

END_SPORTMESH_NAMESPACE
