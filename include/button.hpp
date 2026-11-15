#pragma once

#include <cstdint>

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "errcode.hpp"
#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

/** @brief Button ID and classified press sent to the shared queue. */
struct ButtonEvent {
  std::uint8_t btnId;
  enum Type { kNone, kShort, kLong } type = kNone;
};

/** @brief GPIO and timing settings for hardware-debounced input. */
struct ButtonConfig {
  std::uint8_t pin;                   ///< Button GPIO.
  bool active_low = true;             ///< LOW means pressed.
  bool internal_pullup = false;       ///< Unsupported on GPIO34–39.
  std::uint32_t long_press_ms = 800;  ///< Long-press threshold, ms.
  std::uint8_t btnId = pin;           ///< Event ID; defaults to GPIO.
};

/**
 * @brief Uses GPIO interrupts and a FreeRTOS task to queue press events.
 * @details Short presses are queued on release; long presses once
 * at the threshold. The caller owns and drains the shared queue.
 * @note A started instance must remain alive for the firmware's lifetime.
 */
class Button {
public:
  /** @brief Store configuration; call Start() to begin monitoring. */
  Button(const ButtonConfig& cfg);

  Button(const Button&) = delete;
  Button& operator=(const Button&) = delete;
  Button(Button&&) = delete;
  Button& operator=(Button&&) = delete;

  /**
   * @brief Start monitoring and sending events to the supplied queue.
   * @param events Queue created with sizeof(ButtonEvent) items.
   * Must remain valid for the firmware's lifetime.
   * @return kOk on success, kFail for invalid settings or queue,
   * or kRTOS if task creation fails.
   */
  ErrCode Start(QueueHandle_t events);

  const ButtonConfig& getConfig() const { return m_cfg; }

private:
  /**
   * @brief Notify the task of a GPIO edge.
   * @param param Owning Button instance.
   */
  static void ARDUINO_ISR_ATTR onInputChanged(void* param);
  /**
   * @brief Classify presses and queue events.
   * @param param Owning Button instance.
   */
  static void taskProcessInput(void* param);

  ButtonConfig m_cfg;
  TaskHandle_t m_process_task_handle = nullptr;
  QueueHandle_t m_push_events = nullptr;
};

END_SPORTMESH_NAMESPACE
