#include "station.hpp"

#include "esp_log.h"

// valid adresses in range 0001..7FFF
static constexpr lm::AddressType kInvalidAddress = 0;
static constexpr const char* kTag = "station";

#define IS_ADDR_VALID(addr) \
  (addr != kInvalidAddress)

Station::Station(lm::PinConfig pin_config, 
                 lm::RadioConfig radio_config,
                 StationType station_type)

  : base_station_addr{ kInvalidAddress }
{

  lm::LoRaMeshProtocolConfig protocol_config;
  protocol_config.setNodeCapabilities(station_type);
  
  m_mesher = lm::LoraMesher::Builder()
                 .withRadioConfig(radio_config)
                 .withPinConfig(pin_config)
                 .withLoRaMeshProtocol(protocol_config)
                 .Build();

  m_mesher->SetDataCallback(
    [this](lm::AddressType source, const std::vector<uint8_t>& data) {
      m_data_callback(source, data); 
    }
  );

}

int Station::start()
{
  lm::Result init_result = m_mesher->Start();
  if (!init_result) {
    ESP_LOGE("station", "%s", init_result.GetErrorMessage().c_str());
    return 1;
  }
  return 0;
}

int Station::register_punch(PunchMessage msg)
{
  if (!IS_ADDR_VALID(base_station_addr)) {
    ESP_LOGW(kTag, "base station undiscovered");
    return 1;
  }
  
  auto capatabilities = m_mesher->GetNodeCapabilities();
  if (capatabilities & StationType::kBaseStation 
      || capatabilities & StationType::kCheckpoint)
    return 0;
  
  std::vector<uint8_t> data;
  msg.serialize(data);
  lm::Result res = m_mesher->Send(base_station_addr, data);

  if (!res) {
    ESP_LOGE(kTag, "%s", res.GetErrorMessage().c_str());
    return 1;
  }

  return 0;
}

void Station::m_discover_base_station()
{
  auto routes = m_mesher->GetRoutingTable();

  if (m_mesher->GetNodeCapabilities() & StationType::kBaseStation)
    return;

  if (routes.empty()) {
    ESP_LOGW(kTag, "No routes in table");
    return;
  }

  auto base = 
    m_mesher->GetClosestNodeByCapability(StationType::kBaseStation);

  if (!base.has_value()) {
    base_station_addr = kInvalidAddress;
    ESP_LOGW(kTag, "No base station in route table");
  }

  base_station_addr = base.value().destination;
  ESP_LOGI(kTag, "Discovered base station %x", base_station_addr);
}

void Station::m_data_callback(lm::AddressType source, 
                              const std::vector<uint8_t>& data) {

  ESP_LOGI(kTag, "Received data from: 0x%X (%zu bytes)",
           static_cast<unsigned int>(source), data.size());

  if (data.empty()) {
      ESP_LOGI(kTag, "(no data)");
  } else {
      ESP_LOG_BUFFER_HEX_LEVEL(kTag, data.data(), data.size(), ESP_LOG_INFO);
  }
}
