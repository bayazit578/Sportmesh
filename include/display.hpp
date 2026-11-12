#pragma once

#include <cstdint>
#include <concepts>
#include <cstdarg>

#include <TM16xxDisplay.h>
#include <TM1650.h>
#include <TM1637.h>

#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

template<typename T>
concept DisplayType 
  = std::same_as<T, TM1650> 
    || std::same_as<T, TM1637>;

struct DisplayConfig
{
  std::uint8_t pin_data;
  std::uint8_t pin_clk;
  std::uint8_t digit_cnt;
};

template<DisplayType T>
class Display
{
public:
  Display(DisplayConfig cfg);

  Display(Display&) = delete;
  Display& operator=(const Display&) = delete;

  Display(Display&&) noexcept = delete;
  Display& operator=(Display&&) noexcept = delete;

  void Print(const char* fmt, ...)
    __attribute__((format(printf, 2, 3)));

private:

  T m_display_module;
  TM16xxDisplay m_display;

  // @param param Display pointer
  template<DisplayType Tt>
  friend void taskPrint(void* param);
};


template<DisplayType T>
Display<T>::Display(DisplayConfig cfg )
  : m_display_module{ cfg.pin_data, cfg.pin_clk },
    m_display{ &m_display_module, cfg.digit_cnt }
{
}

template<DisplayType T>
void Display<T>::Print(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  m_display.vprintf(fmt, args);
  va_end(args);
}

template<DisplayType T>
void taskPrint(void* param)
{
  Display<T>* display = static_cast<Display<T>*>(param);
}

END_SPORTMESH_NAMESPACE
