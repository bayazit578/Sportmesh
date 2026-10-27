#include "station.hpp"

#include "esp_log.h"

Station::Station(lm::PinConfig pin_config, 
                 lm::RadioConfig radio_config,
                 StationType station_type)
{

  m_protocol_config.setNodeCapabilities(station_type);
  
  m_mesher = lm::LoraMesher::Builder()
                 .withRadioConfig(radio_config)
                 .withPinConfig(pin_config)
                 .withLoRaMeshProtocol(m_protocol_config)
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

int Station::register_punch()
{
  
}

void Station::m_discover_base_station()
{
  auto routes = m_mesher->GetRoutingTable();

  if (m_protocol_config.getNodeCapabilities() & StationType::kBaseStation)
    return;

  if (routes.empty()) {
    ESP_LOGW(kTag, "No routes in table");
    return;
  }

  for (auto& route : routes) {
    std::uint8_t cap = 
      m_mesher->GetNodeCapabilities(route.destination);

    if (cap & StationType::kBaseStation) {
      base_station_addr = route.destination;
      ESP_LOGI(kTag, "Discovered base station %d", route.destination);
      return;
    }
  }
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
