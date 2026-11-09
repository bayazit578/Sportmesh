#include "display.hpp"
#include "config.hpp"

#include <TM1650.h>
#include <cstdarg>


template<DisplayType T>
Display<T>::Display(std::uint8_t pin_data, 
                    std::uint8_t pin_clk, 
                    std::uint8_t digit_cnt)
  : m_display_module{ pin_data, pin_clk },
    m_display{ &m_display_module, digit_cnt }
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
