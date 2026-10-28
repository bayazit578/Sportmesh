#include "station.hpp"
#include "config.hpp"
#include "reader.hpp"

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

  if (int ret = station.Start(); ret != 0) { }

  Reader reader { station.getReadHandler(),
                  station.getReadQueue() };

  for (;;) { }
}

void loop() { }
