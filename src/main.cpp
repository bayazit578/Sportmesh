#include "display.hpp"
#include "station_controller.hpp"
#include "config.hpp"

void setup() {

  Serial.begin(SERIAL_BAUDRATE);

  loramesher::PinConfig pin_config {
    SM::config::kPinCs,
    SM::config::kPinRst,
    SM::config::kPinDio0,
    SM::config::kPinBusy,
    SM::config::kPinSck,
    SM::config::kPinMiso,
    SM::config::kPinMosi
  };

  loramesher::RadioConfig radio_config {
    SM::config::kRadioType,
    SM::config::kFrequency,       
    SM::config::kSpreadingFactor, 
    SM::config::kBandwidth,       
    SM::config::kCodingRate,      
    SM::config::kPower,           
    SM::config::kSyncWord,        
    SM::config::kCrc,             
    SM::config::kPreambleLength
  };

  SM::DisplayConfig display_config {
    SM::config::kI2cDio,
    SM::config::kI2cClk,
    SM::config::kDisplayDigitCnt
  };

  SM::StationController controller {
    pin_config,
    radio_config,
    display_config
  };

  for (;;) {  }
}

void loop() { }
