#include "station.hpp"
#include "config.hpp"

void setup() {

  Serial.begin(SERIAL_BAUDRATE);

  loramesher::PinConfig pin_config {
    config::kPinCs,
    config::kPinRst,
    config::kPinIrq,
    config::kPinBusy,
    config::kPinSck,
    config::kPinMiso,
    config::kPinMosi
  };

  loramesher::RadioConfig radio_config {
    config::kRadioType,
    config::kFrequency,       
    config::kSpreadingFactor, 
    config::kBandwidth,       
    config::kCodingRate,      
    config::kPower,           
    config::kSyncWord,        
    config::kCrc,             
    config::kPreambleLength,  
  };

  Station station { pin_config, radio_config };

  if (int ret = station.start(); ret != 0) { }

  
}

void loop() { }
