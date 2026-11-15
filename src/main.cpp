#include "button.hpp"
#include "display.hpp"
#include "errcode.hpp"
#include "reader.hpp"
#include "station_controller.hpp"
#include "config.hpp"

static constexpr const char* kTag = "main";

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

  SM::ReaderConfig reader_config {
    SM::config::kReaderPinIrq,
    SM::config::kReaderPinRst,
    SM::config::kReaderPinSck,
    SM::config::kReaderPinMiso,
    SM::config::kReaderPinMosi,
    SM::config::kReaderPinCs,
  };

  SM::ButtonConfig right_button_config {
    SM::config::kBtn1Pin
  };

  SM::ButtonConfig left_button_config {
    SM::config::kBtn2Pin
  };

  SM::StationController controller {
    pin_config,
    radio_config,
    reader_config,
    display_config,
    left_button_config,
    right_button_config
  };

  if (SM::ErrCode code = controller.Start(); 
      code != SM::ErrCode::kOk) {
    ESP_LOGE(kTag, "Failed to start controller");
    return;
  }

  for (;;) { 
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void loop() { }
