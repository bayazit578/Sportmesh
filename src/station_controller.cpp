#include "station_controller.hpp"

#include "sm_namespace.hpp"
#include "station.hpp"

BEGIN_SPORTMESH_NAMESPACE

StationController::StationController(loramesher::PinConfig lora_pin_cfg,
                                     loramesher::RadioConfig lora_radio_cfg,
                                     DisplayConfig display_cfg)
  : m_station { lora_pin_cfg, lora_radio_cfg, kCheckpoint },
    m_reader { m_station.getReadHandler(), m_station.getReadQueue() },
    m_display { display_cfg }
{
  if (ErrCode ret = m_station.Start(); ret != ErrCode::kOk) { }
}

END_SPORTMESH_NAMESPACE
