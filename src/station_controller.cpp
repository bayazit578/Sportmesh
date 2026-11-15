#include "station_controller.hpp"

#include "freertos/projdefs.h"
#include "sm_namespace.hpp"
#include "station.hpp"

BEGIN_SPORTMESH_NAMESPACE

#define START_CHECK_OR_FAIL(obj, ...) \
  if (ErrCode ret = obj.Start(__VA_ARGS__); ret != ErrCode::kOk) { }

static constexpr std::size_t kBtnEventQueueSize = 10;
static constexpr const char* kTag = "Controller";

void taskMainLoop(void* param);

StationController::StationController(
  const loramesher::PinConfig& lora_pin_cfg,
  const loramesher::RadioConfig& lora_radio_cfg,
  const ReaderConfig& reader_cfg,
  const DisplayConfig& display_cfg,
  const ButtonConfig& left_buttong_cfg,
  const ButtonConfig& right_button_cfg)

  : m_station { lora_pin_cfg, 
                lora_radio_cfg, 
                kCheckpoint },
    m_reader { reader_cfg },
    m_display { display_cfg },
    m_lbutton { left_buttong_cfg },
    m_rbutton { right_button_cfg },
    m_btn_event_queue { nullptr },
    m_main_loop_task_handle { nullptr }
{
}

ErrCode StationController::Start()
{
  START_CHECK_OR_FAIL(m_station);

  START_CHECK_OR_FAIL(m_reader, 
                      m_station.getReadHandler(), 
                      m_station.getReadQueue());

  m_btn_event_queue = xQueueCreate(
    kBtnEventQueueSize, 
    sizeof(ButtonEvent));

  if (!m_btn_event_queue) {
    ESP_LOGE(
      kTag, 
      "Failed to create button event queue");
    return ErrCode::kRTOS;
  }

  START_CHECK_OR_FAIL(m_lbutton, m_btn_event_queue);
  START_CHECK_OR_FAIL(m_rbutton, m_btn_event_queue);

  return ErrCode::kOk;
}

ErrCode StationController::CreateMainLoopTask()
{
  auto res = xTaskCreate(
    taskMainLoop, "register punch",
    512, this,
    1, &m_main_loop_task_handle);

  if (res != pdPASS) {
    ESP_LOGE(kTag, "Failed to create main loop task");
    return ErrCode::kRTOS;
  }

  return ErrCode::kOk;
}

void taskMainLoop(void* param)
{
  StationController* st_controller = 
    static_cast<StationController*>(param);

  ButtonEvent event{};

  for (;;) {
    if (xQueueReceive(st_controller->m_btn_event_queue,
                      &event, pdMS_TO_TICKS(10)) != pdPASS) {
      vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(
      kTag, 
      "Button id: %d, type: %d", 
      event.btn_id,
      event.type);
  }
}

END_SPORTMESH_NAMESPACE
