#include "station_controller.hpp"

#include "freertos/projdefs.h"
#include "sm_namespace.hpp"
#include "station.hpp"

BEGIN_SPORTMESH_NAMESPACE

#define START_CHECK_ERR_OR_RETURN(obj, ...)                      \
  if (ErrCode ret = obj.Start(__VA_ARGS__); ret != ErrCode::kOk) \
  {                                                              \
    m_display.Printf("%d", static_cast<int>(ret));               \
    return ret;                                                  \
  }

#define CHECK_ERR_OR_RETURN(expr) \
  if (expr != ErrCode::kOk)   \
    return expr;

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
                kBaseStation },
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
  START_CHECK_ERR_OR_RETURN(m_station);

  if (m_station.getType() == kCheckpoint) {
    START_CHECK_ERR_OR_RETURN(
      m_reader,
      m_station.getReadHandler(),
      m_station.getReadQueue());
  }

  m_btn_event_queue = xQueueCreate(
    kBtnEventQueueSize, 
    sizeof(ButtonEvent));

  if (!m_btn_event_queue) {
    ESP_LOGE(
      kTag, 
      "Failed to create button event queue");
    return ErrCode::kRTOS;
  }

  START_CHECK_ERR_OR_RETURN(m_lbutton, m_btn_event_queue);
  START_CHECK_ERR_OR_RETURN(m_rbutton, m_btn_event_queue);

  CHECK_ERR_OR_RETURN(CreateMainLoopTask());

  m_display.Printf(CurrentTypeToString());

  return ErrCode::kOk;
}

void StationController::MenuSelectNext()
{
  StationType current = m_station.getType(); 
  StationType next;

  if (current == kBaseStation)  next = kCheckpoint;
  if (current == kCheckpoint)   next = kRetranslator;
  if (current == kRetranslator) next = kBaseStation;

  m_station.ChangeType(next);

  m_display.Printf(CurrentTypeToString());
}

void StationController::MenuSelectPrevious()
{
  StationType current = m_station.getType(); 
  StationType next;

  if (current == kBaseStation)  next = kRetranslator;
  if (current == kCheckpoint)   next = kBaseStation;
  if (current == kRetranslator) next = kCheckpoint;

  m_station.ChangeType(next);

  m_display.Printf(CurrentTypeToString());
}

const char* StationController::CurrentTypeToString()
{
  switch (m_station.getType())
  {
    case kBaseStation:  return "base";
    case kCheckpoint:   return "chpt";
    case kRetranslator: return "rtlr";
    default:            return "none";
  }
}

ErrCode StationController::CreateMainLoopTask()
{
  auto res = xTaskCreate(
    taskMainLoop, "register punch",
    4096, this,
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

  int left_btn_id = 
    st_controller->m_lbutton.getConfig().btnId;

  int right_btn_id = 
    st_controller->m_rbutton.getConfig().btnId;

  for (;;) {
    // FIXME
    if (xQueueReceive(st_controller->m_btn_event_queue,
                      &event, pdMS_TO_TICKS(10)) != pdPASS) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    if (event.btnId == left_btn_id)
        switch (event.type) {
          case ButtonEvent::kShort:
            st_controller->MenuSelectPrevious();
            break;

          case ButtonEvent::kLong:
            // st_controller->MenuGoBack();
            break;

          default:
            break;
        }

    else if (event.btnId == right_btn_id)
        switch (event.type) {
          case ButtonEvent::kShort:
            st_controller->MenuSelectNext();
            break;

          case ButtonEvent::kLong:
            // st_controller->MenuConfirm();
            break;

          default:
            break;
        }
  }
}

END_SPORTMESH_NAMESPACE
