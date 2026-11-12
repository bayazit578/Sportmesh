#pragma once

#include "display.hpp"
#include "station.hpp"
#include "reader.hpp"
#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

class StationController
{
public:
  StationController(loramesher::PinConfig lora_pin_cfg,
                    loramesher::RadioConfig lora_radio_cfg,
                    DisplayConfig display_cfg);

  StationController(StationController&) = delete;
  StationController& operator=(const StationController&) = delete;

  StationController(StationController&&) noexcept = delete;
  StationController& operator=(StationController&&) noexcept = delete;

private:
  Station m_station;
  Reader  m_reader;

  Display<TM1650> m_display;
};

END_SPORTMESH_NAMESPACE
