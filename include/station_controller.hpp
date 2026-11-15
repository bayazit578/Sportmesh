#pragma once

#include "button.hpp"
#include "display.hpp"
#include "freertos/idf_additions.h"
#include "station.hpp"
#include "reader.hpp"
#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

class StationController
{
public:
  StationController(const loramesher::PinConfig& lora_pin_cfg,
                    const loramesher::RadioConfig& lora_radio_cfg,
                    const ReaderConfig& reader_cfg,
                    const DisplayConfig& display_cfg,
                    const ButtonConfig& left_buttong_cfg,
                    const ButtonConfig& right_button_cfg);

  StationController(StationController&) = delete;
  StationController& operator=(const StationController&) = delete;

  StationController(StationController&&) noexcept = delete;
  StationController& operator=(StationController&&) noexcept = delete;

  ErrCode Start();

private:

  void MenuSelectPrevious(); 
  void MenuGoBack(); 
  void MenuSelectNext(); 
  void MenuConfirm(); 

  const char* CurrentTypeToString();

  ErrCode CreateMainLoopTask();

  Station m_station;
  Reader  m_reader;

  Display<TM1650> m_display;

  Button m_lbutton, m_rbutton;

  QueueHandle_t m_btn_event_queue;
  TaskHandle_t m_main_loop_task_handle;

  friend void taskMainLoop(void* param);
};

END_SPORTMESH_NAMESPACE
