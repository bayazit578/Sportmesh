#pragma once

#include "station.hpp"
#include "reader.hpp"

enum class StationContollerState 
{
};

class StationController
{

  StationController();

  // Non-copyable
  StationController(StationController&) = delete;
  StationController& operator=(const StationController&) = delete;

  // Movable
  StationController(StationController&&) noexcept = default;
  StationController& operator=(StationController&&) noexcept = default;


private:
  Station station;
  Reader reader;
};
